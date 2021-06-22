/* Examples and Documentation for
 * SmartCGMS - continuous glucose monitoring and controlling framework
 * https://diabetes.zcu.cz/
 *
 * Copyright (c) since 2018 University of West Bohemia.
 *
 * Contact:
 * diabetes@mail.kiv.zcu.cz
 * Medical Informatics, Department of Computer Science and Engineering
 * Faculty of Applied Sciences, University of West Bohemia
 * Univerzitni 8, 301 00 Pilsen
 * Czech Republic
 *
 *
 * Purpose of this software:
 * This software is intended to demonstrate work of the diabetes.zcu.cz research
 * group to other scientists, to complement our published papers. It is strictly
 * prohibited to use this software for diagnosis or treatment of any medical condition,
 * without obtaining all required approvals from respective regulatory bodies.
 *
 * Especially, a diabetic patient is warned that unauthorized use of this software
 * may result into severe injure, including death.
 *
 *
 * Licensing terms:
 * Unless required by applicable law or agreed to in writing, software
 * distributed under these license terms is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

 /*
  * @author = Bc. David Pivovar
  */

#include "pa_detection.h"

CPa_Detection::CPa_Detection(scgms::IFilter* output) : CBase_Filter(output) {
	//
}

CPa_Detection::~CPa_Detection() {
	//
}

HRESULT IfaceCalling CPa_Detection::QueryInterface(const GUID* riid, void** ppvObj) {
	if (Internal_Query_Interface<scgms::IFilter>(cho_detection::id_cho, *riid, ppvObj)) return S_OK;

	return E_NOINTERFACE;
}

HRESULT IfaceCalling CPa_Detection::Do_Configure(scgms::SFilter_Configuration configuration, refcnt::Swstr_list& error_description) {
	if (configuration.Read_Bool(cho_detection::rsSHeartbeat)) {
		signals.push_back(scgms::signal_Heartbeat);
		auto th =  configuration.Read_Double(cho_detection::rsThHeartbeat, 80);
		th_signal.emplace(scgms::signal_Heartbeat, th);
	}
	if (configuration.Read_Bool(cho_detection::rsSSteps)) {
		signals.push_back(scgms::signal_Steps);
		auto th = configuration.Read_Double(cho_detection::rsThSteps, 20);
		th_signal.emplace(scgms::signal_Steps, th);
	}
	if (configuration.Read_Bool(cho_detection::rsSAcc)) {
		signals.push_back(scgms::signal_Acceleration);
		auto th = configuration.Read_Double(cho_detection::rsThAcc, 1.1);
		th_signal.emplace(scgms::signal_Acceleration, th);
	}
	if (configuration.Read_Bool(cho_detection::rsSEl)) {
		signals.push_back(scgms::signal_Electrodermal_Activity);
		auto th = configuration.Read_Double(cho_detection::rsThEl, 10);
		th_signal.emplace(scgms::signal_Electrodermal_Activity, th);
	}

	if (b_mean = configuration.Read_Bool(cho_detection::rsMean) || b_class) {
		mean_window = configuration.Read_Int(cho_detection::rsMeanSize);
		if (mean_window < 1) {
			error_description.push(L"Window size must be at least 1!");
			return E_INVALIDARG;
		}
	}
	else {
		mean_window = 1;
	}

	if (configuration.Read_Bool(cho_detection::rsDesc)) {
		ist_signal = configuration.Read_GUID(cho_detection::rsSignal, cho_detection::signal_savgol);
		ist_window = configuration.Read_Int(cho_detection::rsWindowSize);
		if (ist_window < 1) {
			error_description.push(L"Window size must be at least 1!");
			return E_INVALIDARG;
		}

		std::vector<double> lb, def, ub;
		if (!configuration.Read_Parameters(cho_detection::rsThresholds, lb, def, ub)) {
			error_description.push(L"Cannot read the parameters!");
			return E_INVALIDARG;
		}

		for (size_t i = 0; i < 2; i++) {
			thresholds[i] = def[i * 2];
			weights[i] = def[i * 2 + 1];
		}
	}

	//classification - test purposes only
	if (b_class) {
		classifier = std::make_unique<ml>(class_type, signals.size(), class_path);
	}
	
	return S_OK;
}

HRESULT IfaceCalling CPa_Detection::Do_Execute(scgms::UDevice_Event event) {
	//get segment data
	PASegmentData* data;
	if (event.is_level_event()) {
		auto seg_id = event.segment_id();
		auto it = mSegments.find(seg_id);
		if (it == mSegments.end()) {
			std::map<GUID, swl<double>> values;
			std::map<GUID, SFeatures> features;
			for each (GUID s in signals)
			{
				values.emplace(s, swl<double>(mean_window));
				features.emplace(s, SFeatures());
			}
			swl<double> act(ist_window);
			act.push_front(0);

			PASegmentData data = { -1, false, -1, -1, act, values, features };
			it = mSegments.emplace(seg_id, data).first;
		}
		data = &it->second;
	}

	//detected pa event
	scgms::UDevice_Event event_pa(scgms::NDevice_Event_Code::Level);
	event_pa.device_id() = cho_detection::id_pa;
	event_pa.segment_id() = event.segment_id();
	event_pa.device_time() = event.device_time();
	event_pa.signal_id() = cho_detection::signal_pa;
	event_pa.level() = 0;

	//ist signal
	if (b_edge && event.is_level_event() && event.signal_id() == cho_detection::signal_savgol && event.level() > 0) {
		double act = activation(event, *data);

		//activation event
		scgms::UDevice_Event event_act(scgms::NDevice_Event_Code::Level);
		event_act.device_id() = cho_detection::id_cho;
		event_act.signal_id() = cho_detection::signal_activation;
		event_act.segment_id() = event.segment_id();
		event_act.device_time() = event.device_time();
		event_act.level() = act+20;
		auto rc = mOutput.Send(event_act);
		if (!Succeeded(rc)) {
			return rc;
		}
	}
	
	if (event.is_level_event() && std::find(signals.begin(), signals.end(), event.signal_id()) != signals.end() && event.level() > 0) {
		if (data->last_event_time == -1) data->last_event_time = event.device_time(); //first value set time

		//if there is a space > 5 min between measures add 0
		/*double delta = (event.device_time() - data->last_event_time) / scgms::One_Minute;
		if (delta > 5) {
			for (int i = 0; i < (int)(delta / 5); ++i) {
				data->values[event.signal_id()].push_back(0);
			}
		}*/

		//save values
		auto& values = data->values[event.signal_id()];
		values.push_back(event.level());
		data->features[event.signal_id()] = calc_features(values.to_vector());

		data->last_event_time = event.device_time();

		//threshold
		bool res = true;
		for each (auto signal in data->features)
		{
			//if window_size = 1 then mean = value
			res = res && (signal.second.mean > th_signal[signal.first]);
		}
		if (res) {
			event_pa.level() = 1;
		}

		//confirmation
		if (b_edge) {
			if (data->activation_m.front() < th_edge) {
				event_pa.level() += 1;
			}
		}
		else {
			event_pa.level() *= 2;
		}

		//classification - test purposes only
		if (b_class) {
			auto res = classifier->classify(get_feature_vector(data->features));
			event_pa.level() = res * 2;
		}

		//send detected pa
		auto rc = mOutput.Send(event_pa);
		if (!Succeeded(rc)) {
			return rc;
		}
	}
	
	return mOutput.Send(event);
}

double CPa_Detection::activation(scgms::UDevice_Event& event, PASegmentData& data)
{
	//initializations
	if (!data.initialized) {
		data.initialized = true;
		data.prevL = event.level();
		data.prevT = event.device_time();
		return 0;
	}

	double time = (event.device_time() - data.prevT) / scgms::One_Minute;
	double der = (event.level() - data.prevL) / time;

	//get weight of the signal
	double act_m = 0;
	for (size_t i = 0; i < thresholds.size(); ++i) {
		if (der < -1 * thresholds[i]) act_m = -1 * weights[i];
	}

	if (act_m <= -1 * weights[0]) {
		for (size_t i = 0; i < data.activation_m.size(); ++i) {
			if (data.activation_m.at(i) <= -1 * (th_act + i * 0.2)) {
				act_m -= 0.1 * i;
			}
		}
	}

	//store values
	data.activation_m.push_front(act_m);

	data.prevL = event.level();
	data.prevT = event.device_time();

	return act_m;
}

SFeatures CPa_Detection::calc_features(std::vector<double> data) {
	SFeatures features = SFeatures();

	features.mean = mean(data);
	features.median = median(data);
	features.std = std(data);
	features.quantile = quantile_diff(data);

	return features;
}

std::vector<double> CPa_Detection::get_feature_vector(std::map<GUID, SFeatures> features)
{
	std::vector<double> vec;

	for each (auto var in features)
	{
		auto f = var.second;
		vec.push_back(f.mean);
		vec.push_back(f.median);
		vec.push_back(f.std);
		vec.push_back(f.quantile);
	}

	return vec;
}