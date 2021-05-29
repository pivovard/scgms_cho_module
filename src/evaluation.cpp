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

#include "evaluation.h"
#include "descriptor.h"

CEvaluation::CEvaluation(scgms::IFilter* output) : CBase_Filter(output) {
	//
}

CEvaluation::~CEvaluation() {
	//
}

HRESULT IfaceCalling CEvaluation::QueryInterface(const GUID* riid, void** ppvObj) {
	if (Internal_Query_Interface<scgms::IFilter>(cho_detection::id_savgol, *riid, ppvObj)) return S_OK;

	return E_NOINTERFACE;
}

HRESULT IfaceCalling CEvaluation::Do_Configure(scgms::SFilter_Configuration configuration, refcnt::Swstr_list& error_description) {
	signal_ref = configuration.Read_GUID(cho_detection::rsSignalRef);
	signal_det = configuration.Read_GUID(cho_detection::rsSignalDet);
	
	max_delay = configuration.Read_Int(cho_detection::rsMaxDelay, 180);
	if (max_delay < 0) {
		error_description.push(L"Delay must be non-negative");
		return E_INVALIDARG;
	}
	
	fp_delay = configuration.Read_Int(cho_detection::rsFPDelay, 180);
	if (fp_delay < 0) {
		error_description.push(L"Delay must be non-negative");
		return E_INVALIDARG;
	}

	late_delay = configuration.Read_Int(cho_detection::rsLateDelay, 0);
	if (late_delay < 0) {
		error_description.push(L"Delay must be non-negative");
		return E_INVALIDARG;
	}
	
	return S_OK;
}

HRESULT IfaceCalling CEvaluation::Do_Execute(scgms::UDevice_Event event) {
	
	if (event.is_level_event())
	{
		process_signal(event);
	}
	
	if (event.is_level_event() && event.signal_id() == signal_ref && event.level() > 0) {
		process_reference(event);
	}
	else if (event.is_level_event() && event.signal_id() == signal_det) {
		//skip first 3 hours (36)
		if(data.drop_count++ < th_drop)
		{
			return mOutput.Send(event);
		}

		//check date (if new day and count > 3 save stat)
		double d;
		std::modf(event.device_time(), &d);
		if(d > data.date)
		{
			data.date = d;
			if(data.day.count > 0)
			{
				data.global += data.day;
			}
			data.day = SegmentStatData();
		}
		
		process_detection(event);
	}
	else if (event.event_code() == scgms::NDevice_Event_Code::Time_Segment_Stop) {
			if (data.day.count > 0)
			{
				data.global += data.day;
			}

			double acc_d = (double)data.global.TPd / data.global.count;
			double acc_c = (double)data.global.TPc / data.global.count;
			double delay = data.global.delay / scgms::One_Minute / data.global.count;
			double delay_conf = data.global.delay_conf / scgms::One_Minute / data.global.TPc;

			scgms::UDevice_Event e(scgms::NDevice_Event_Code::Information);
			e.device_id() = cho_detection::id_eval;
			e.signal_id() = Invalid_GUID;
			e.segment_id() = event.segment_id();
			e.device_time() = event.device_time();

			std::wstringstream stream;
			stream << L"CHO count: " << data.global.count << L" Accuracy detection: " << acc_d << L", delay: " << delay << L", TP detected: " << data.global.TPd
				<< L", Accuracy confirmed: " << acc_c << L", confirmation delay: " << delay_conf << L", TP confirmed: " << data.global.TPc
				<< L", FN: " << data.global.FN << L", FP: " << data.global.FPc;
			e.info.set(stream.str().c_str());

			auto rc = mOutput.Send(e);
			if (!Succeeded(rc)) {
				return rc;
			}
	}

	return mOutput.Send(event);
}

void CEvaluation::process_signal(scgms::UDevice_Event& event)
{
	//reset late detection
	auto t = (event.device_time() - data.ref_time) / scgms::One_Minute;
	if (data.ref_time > 0 && t > max_delay) {
		//if cho not detected
		if (!data.bDetected) {
			data.day.FN++;
		}

		data.ref_time = -1;
		data.bDetected = false;
		data.bConfirmed = false;
		data.detect_time = 0;
	}
	//FP
	t = (event.device_time() - data.fp_time) / scgms::One_Minute;
	if (data.ref_time == -1 && data.fp_time > 0 && !data.bFP  && t > late_delay) {
		data.day.FPc++;
		data.bFP = true;
	}
	//reset FP timer
	if (data.fp_time > 0 && t > fp_delay) {
		data.fp_time = -1;
		data.bFP = false;
	}
}

void CEvaluation::process_reference(scgms::UDevice_Event& event)
{
	//cho not detected
	if (data.ref_time > 0 && !data.bDetected) {
		data.day.FN++;
	}

	//set new cho
	data.day.count++;
	data.ref_time = event.device_time();
	data.bDetected = false;
	data.bConfirmed = false;

	//check detection beforehand
	auto t = (event.device_time() - data.fp_time) / scgms::One_Minute;
	if (!data.bFP && data.fp_time > 0 && t <= late_delay) {
		data.bDetected = true;
		data.day.TPd++;
		data.detect_time = data.fp_time;
	}
}

void CEvaluation::process_detection(scgms::UDevice_Event& event)
{
	//cho detected
	if (event.level() >= th_detection) {
		//detected (increment TP only once)
		if (data.ref_time > 0 && !data.bDetected) {
			data.bDetected = true;
			data.day.TPd++;
			data.day.delay += event.device_time() - data.ref_time;
			data.detect_time = event.device_time();
		}
	}
	//cho confirmed
	if (event.level() >= th_confirmation) {
		if (data.ref_time > 0 && !data.bConfirmed) {
			data.bConfirmed = true;
			data.day.TPc++;
			data.day.delay_conf += event.device_time() - data.detect_time;
		}
		//false detection (or late)
		if (data.ref_time == -1 && data.fp_time == -1) {
			data.fp_time = event.device_time();
			//data.day.FPc++;
		}
	}
}

::SegmentStatData& SegmentStatData::operator+=(const SegmentStatData& day)
{
	count += day.count;
	TPd += day.TPd;
	TPc += day.TPc;
	FN += day.FN;
	FPd += day.FPd;
	FPc += day.FPc;
	delay += day.delay;
	delay_conf += day.delay_conf;

	return *this;
}