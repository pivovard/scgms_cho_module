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

#include "savgol_filter.h"
#include "descriptor.h"

CSavgol_Filter::CSavgol_Filter(scgms::IFilter* output) : CBase_Filter(output) {
	//
}

CSavgol_Filter::~CSavgol_Filter() {
	//
}

HRESULT IfaceCalling CSavgol_Filter::QueryInterface(const GUID* riid, void** ppvObj) {
	if (Internal_Query_Interface<scgms::IFilter>(detection::id_savgol, *riid, ppvObj)) return S_OK;

	return E_NOINTERFACE;
}

HRESULT IfaceCalling CSavgol_Filter::Do_Configure(scgms::SFilter_Configuration configuration, refcnt::Swstr_list& error_description) {
	input_signal = configuration.Read_GUID(detection::rsSignal, scgms::signal_IG);
	
	window = configuration.Read_Int(detection::rsSavgolWindow, 21);
	degree = configuration.Read_Int(detection::rsSavgolDeg, 3);

	if (window < 1) {
		error_description.push(L"Size of the window must be at least 1!");
		return E_INVALIDARG;
	}
	if (degree < 1) {
		error_description.push(L"Degree must be at least 1!");
		return E_INVALIDARG;
	}
	if (window <= degree) {
		error_description.push(L"Size of the window must be greater than degree!");
		return E_INVALIDARG;
	}

	return S_OK;
}

HRESULT IfaceCalling CSavgol_Filter::Do_Execute(scgms::UDevice_Event event) {
	
	if (event.is_level_event() && event.signal_id() == input_signal) {
		
		auto seg_id = event.segment_id();
		auto it = mSegments.emplace(seg_id, swl<double>(3 * window));

		auto rc = process(event, it.first->second);
		if (!Succeeded(rc)) {
			return rc;
		}
	}
	else if (event.event_code() == scgms::NDevice_Event_Code::Time_Segment_Stop) {
		mSegments.erase(event.segment_id());
	}

	return mOutput.Send(event);
}

HRESULT CSavgol_Filter::process(scgms::UDevice_Event &event, swl<double>& ist)
{
	double _ist = 0;
	ist.push_back(event.level());

	auto vec = std::vector<double>(ist.begin(), ist.end());
	//_ist = (sg_smooth(vec, window, degree).back());
	auto vec2 = sg_smooth(vec, window, degree);
	//_ist = vec2[(int)(window / 2)]; //-2 => value before last one

	//send smoothed signal
	scgms::UDevice_Event e(scgms::NDevice_Event_Code::Level);
	e.device_id() = detection::id_savgol;
	e.signal_id() = detection::signal_savgol;
	e.segment_id() = event.segment_id();
	e.device_time() = event.device_time();
	e.level() = _ist;

	return mOutput.Send(e);
}
