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

SFeatures CPa_Detection::calc_features(std::vector<double> data) {
	SFeatures features = SFeatures();

	features.mean = mean(data);
	features.median = median(data);
	features.std = std(data);
	features.quantile = quantile_diff(data);

	return features;
}

HRESULT IfaceCalling CPa_Detection::Do_Configure(scgms::SFilter_Configuration configuration, refcnt::Swstr_list& error_description) {
	
	if (bHeart) signals.push_back(scgms::signal_Heartbeat);
	if (bSteps) signals.push_back(scgms::signal_Steps);
	if (bElectro) signals.push_back(scgms::signal_Electrodermal_Activity);
	if (bTemp) signals.push_back(scgms::signal_Skin_Temperature);
	if (bAcc) signals.push_back(scgms::signal_Acceleration);

	return S_OK;
}

HRESULT IfaceCalling CPa_Detection::Do_Execute(scgms::UDevice_Event event) {

	if (event.is_level_event() && std::find(signals.begin(), signals.end(), event.signal_id()) != signals.end() && event.level() > 0) {
		//get segment data
		auto seg_id = event.segment_id();
		auto it = mSegments.find(seg_id);
		if (it == mSegments.end()) {
			std::map<GUID, swl<double>> values;
			std::map<GUID, SFeatures> features;
			for each (GUID s in signals)
			{
				values.emplace(s, swl<double>(window_size));
				features.emplace(s, SFeatures());
			}

			PASegmentData data = { -1, values, features };
			it = mSegments.emplace(seg_id, data).first;
		}

		auto data = it->second;

		if (data.last_event_time == -1) data.last_event_time = event.device_time(); //first value

		//if there is a space > 5 min between measures add 0
		double time = (event.device_time() - data.last_event_time) / scgms::One_Minute;
		if (time > 5) {
			for (int i = 0; i < (int)(time / 5); ++i) {
				data.values[event.signal_id()].push_back(0);
			}
		}

		auto values = data.values[event.signal_id()];
		values.push_back(event.level());
		data.features[event.signal_id()] = calc_features(values.to_vector());


	}

	

	
	/*if (event.is_level_event() && event.signal_id() == scgms::signal_Acceleration && event.level() > 0) {
		//get segment data
		auto seg_id = event.segment_id();
		auto it = mSegments.find(seg_id);
		if (it == mSegments.end()) {
			swl<double> data(window_size);
			it = mSegments.emplace(seg_id, data).first;
		}

		swl<double> &data = it->second;
		data.push_back(event.level());
		
		if(data.size() < window_size) {
			return mOutput.Send(event);
		}
		
		auto quantiles = quantile<double>(data, { 0.25, 0.75 });
		double diff = quantiles[1] - quantiles[0];

		//send detected pa
		scgms::UDevice_Event event_pa(scgms::NDevice_Event_Code::Level);
		event_pa.device_id() = cho_detection::id_pa;
		event_pa.segment_id() = event.segment_id();
		event_pa.device_time() = event.device_time();
		event_pa.signal_id() = cho_detection::signal_pa;
		if (diff > th_acc) {
			event_pa.level() = 5;
			pa_detected_count++;
		}
		else {
			event_pa.level() = 0;
		}

		auto rc = mOutput.Send(event_pa);
		if (!Succeeded(rc)) {
			return rc;
		}
	}
	else if (event.event_code() == scgms::NDevice_Event_Code::Time_Segment_Stop) {
		mSegments.erase(event.segment_id());

		// scgms::UDevice_Event e(scgms::NDevice_Event_Code::Information);
		// e.device_id() = cho_detection::id_pa;
		// e.signal_id() = Invalid_GUID;
		// e.segment_id() = event.segment_id();
		// e.device_time() = event.device_time();
		// e.info.set(itopa_detected_count);
		//
		// auto rc = mOutput.Send(e);
		// if (!Succeeded(rc)) {
		// 	return rc;
		// }
	}*/
	
	return mOutput.Send(event);
}