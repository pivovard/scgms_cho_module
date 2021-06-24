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

#include "cho_detection.h"

CCho_Detection::CCho_Detection(scgms::IFilter *output) : CBase_Filter(output) {
	//
}

CCho_Detection::~CCho_Detection() {
	//
}

HRESULT IfaceCalling CCho_Detection::QueryInterface(const GUID*  riid, void ** ppvObj) {
	if (Internal_Query_Interface<scgms::IFilter>(detection::id_cho, *riid, ppvObj)) return S_OK;

	return E_NOINTERFACE;
}

std::vector<double> split(const std::wstring& ws, wchar_t delim) {
	std::vector<double> vec;
	std::wstringstream wss(ws);
	std::wstring number;
	while (std::getline(wss, number, delim)) {
		vec.push_back(std::stod(number));
	}
	return vec;
}

HRESULT IfaceCalling CCho_Detection::Do_Configure(scgms::SFilter_Configuration configuration, refcnt::Swstr_list& error_description) {
	input_signal = configuration.Read_GUID(detection::rsSignal, detection::signal_savgol);
	window_size = configuration.Read_Int(detection::rsWindowSize, 12);
	if(window_size < 1){
		error_description.push(L"Window size must be at least 1!");
		return E_INVALIDARG;
	}

	std::vector<double> lb, def, ub;
	if (!configuration.Read_Parameters(detection::rsThresholds, lb, def, ub)) {
		error_description.push(L"Cannot read the parameters!");
		return E_INVALIDARG;
	}

	for (size_t i = 0; i < 2; i++) {
		thresholds[i] = def[i * 2];
		weights[i] = def[i * 2 + 1];
	}
	
	th_act = configuration.Read_Double(detection::rsThAct, 2);
	if (th_act < 0) {
		error_description.push(L"Activation threshold must be non-negative");
		return E_INVALIDARG;
	}
	
	detect_edges = configuration.Read_Bool(detection::rsEdges);
	detect_desc = configuration.Read_Bool(detection::rsDesc);
	use_rnn = configuration.Read_Bool(detection::rsRnn);
	
	if(use_rnn)
	{
		auto path = configuration.Read_File_Path(detection::rsModelPath);
		if(!std::filesystem::exists(path)){
			error_description.push(L"RNN model file doesn't exists!");
			return E_INVALIDARG;
		}
		rnn::load_model(path);

		th_rnn = configuration.Read_Double(detection::rsRnnThreshold);
		if (th_rnn < 0) {
			error_description.push(L"RNN threshold must be non-negative");
			return E_INVALIDARG;
		}
	}
	
	return S_OK;
}

HRESULT IfaceCalling CCho_Detection::Do_Execute(scgms::UDevice_Event event) {

	if (event.is_level_event() && event.signal_id() == input_signal) {
		//get segment data
		auto seg_id = event.segment_id();
		auto it = mSegments.find(seg_id);
		if (it == mSegments.end()) {
			CHOSegmentData data = { false, -1, -1, swl<double>(window_size), swl<double>(window_size) };
			it = mSegments.emplace(seg_id, data).first;
		}

		//activation event
		scgms::UDevice_Event event_act(scgms::NDevice_Event_Code::Level);
		event_act.device_id() = detection::id_cho;
		event_act.signal_id() = detection::signal_activation;
		event_act.segment_id() = event.segment_id();
		event_act.device_time() = event.device_time();

		//event of detected cho
		scgms::UDevice_Event event_cho(scgms::NDevice_Event_Code::Level);
		event_cho.device_id() = detection::id_cho;
		event_cho.segment_id() = event.segment_id();
		event_cho.device_time() = event.device_time();
		event_cho.signal_id() = detection::signal_cho;
		event_cho.level() = 0;

		double act = 0;
		if (detect_edges) {
			//calc activation
			act = activation(event, it->second);

			//send activation
			event_act.level() = act;
			auto rc = mOutput.Send(event_act);
			if (!Succeeded(rc)) {
				return rc;
			}

			if (!use_rnn && act > th_high) { //only without RNN 
				event_cho.level() = 2;
			}
			else if (act > th_low) {
				event_cho.level() = 1;
			}
		}

		if(use_rnn)
		{
			auto it_rnn = rnnSegments.find(seg_id);
			if (it_rnn == rnnSegments.end()) {
				it_rnn = rnnSegments.emplace(seg_id, rnn(24, 3)).first;
			}
			rnn& rnn = it_rnn->second;

			float res = rnn.predict((event));
			if (res > th_rnn) {
				if (detect_edges) { //confirmation for edges
					event_cho.level() += 1;
				}
				else { //use only RNN
					event_cho.level() = 2;
				}
			}

			//send activation
			if (!detect_edges) {
				event_act.level() = res;
				auto rc = mOutput.Send(event_act);
				if (!Succeeded(rc)) {
					return rc;
				}
			}
		}

		auto rc = mOutput.Send(event_cho);
		if (!Succeeded(rc)) {
			return rc;
		}
	}
	else if (event.event_code() == scgms::NDevice_Event_Code::Time_Segment_Stop){
			mSegments.erase(event.segment_id());
	}

	return mOutput.Send(event);
}

double CCho_Detection::activation(scgms::UDevice_Event& event, CHOSegmentData& data)
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
	double act = 0;
	double act_m = 0;
	for (size_t i = 0; i < thresholds.size(); ++i) {
		if (der > thresholds[i]) act = weights[i];
		if (der < -1 * thresholds[i]) act_m = -1 * weights[i];
	}

	//ascending edge
	if (act >= weights[0]) {
		for (size_t i = 0; i < data.activation.size(); ++i) {
			if (data.activation.at(i) >= th_act + 0.2 * i) {
				act += 0.1 * i;
			}
		}
	}
	//descending edge
	else if (detect_desc && act_m <= -1 * weights[0]) {
		if (data.activation.size() > gap_size) {
			act = *std::max_element(data.activation.begin(), data.activation.begin() + gap_size);
		}
		else {
			act = *std::max_element(data.activation.begin(), data.activation.end());
		}

		for (size_t i = 0; i < data.activation_m.size(); ++i) {
			if (data.activation_m.at(i) <= -1 * (th_act + i * 0.2)) {
				act -= 0.1 * i;
				act_m -= 0.1 * i;
			}
		}
	}

	//store values
	data.activation.push_front(act);
	data.activation_m.push_front(act_m);

	data.prevL = event.level();
	data.prevT = event.device_time();
	
	return act;
}
