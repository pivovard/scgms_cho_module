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
  * Depreceted!
  * 
  * @author = Bc. David Pivovar
  */

#include "lstm_filter.h"
#include "descriptor.h"

CLstm_filter::CLstm_filter(scgms::IFilter* output) : CBase_Filter(output) {
	//
}

CLstm_filter::~CLstm_filter() {
	//
}

HRESULT IfaceCalling CLstm_filter::QueryInterface(const GUID* riid, void** ppvObj) {
	if (Internal_Query_Interface<scgms::IFilter>(cho_detection::id_savgol, *riid, ppvObj)) return S_OK;

	return E_NOINTERFACE;
}

HRESULT IfaceCalling CLstm_filter::Do_Configure(scgms::SFilter_Configuration configuration, refcnt::Swstr_list& error_description) {
	rnn::load_model(L"model/575_fdeep_model.json");
	return S_OK;
}

HRESULT IfaceCalling CLstm_filter::Do_Execute(scgms::UDevice_Event event) {

	if (event.is_level_event() && event.signal_id() == scgms::signal_IG) {

		auto seg_id = event.segment_id();
		auto it = mSegments.find(seg_id);
		if (it == mSegments.end()) {
			it = mSegments.emplace(seg_id, rnn(24, 3)).first;
		}
		rnn& lstm = it->second;

		float res = lstm.predict((event));
		if(res > 0)
		{
			scgms::UDevice_Event event_act(scgms::NDevice_Event_Code::Level);
			event_act.device_id() = cho_detection::id_lstm;
			event_act.signal_id() = cho_detection::signal_activation;
			event_act.segment_id() = event.segment_id();
			event_act.device_time() = event.device_time();
			event_act.level() = res;
			
			auto rc = mOutput.Send(event_act);
			if (!Succeeded(rc)) {
				return rc;
			}

			scgms::UDevice_Event event_cho(scgms::NDevice_Event_Code::Level);
			event_cho.device_id() = cho_detection::id_lstm;
			event_cho.segment_id() = event.segment_id();
			event_cho.device_time() = event.device_time();
			event_cho.signal_id() = cho_detection::signal_cho;
			if (res > threshold) {
				event_cho.level() = 2;
			}
			else
			{
				event_cho.level() = 0;
			}

			rc = mOutput.Send(event_cho);
			if (!Succeeded(rc)) {
				return rc;
			}
		}
	}
	else if (event.event_code() == scgms::NDevice_Event_Code::Time_Segment_Stop) {
		mSegments.erase(event.segment_id());
	}

	return mOutput.Send(event);
}