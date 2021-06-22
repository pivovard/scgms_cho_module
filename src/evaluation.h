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

#pragma once

#include <rtl/FilterLib.h>
#include <rtl/referencedImpl.h>
#include <rtl/UILib.h>

#include <sstream>

#include "swl.h"


#pragma warning( push )
#pragma warning( disable : 4250 ) // C4250 - 'class1' : inherits 'class2::member' via dominance

struct StatisticsData
{
	size_t count = 0;
	
	size_t TPd = 0; //unconfirmed
	size_t TPc = 0; //confirmed
	size_t FN = 0;
	size_t FPd = 0;
	size_t FPc = 0;

	double delay = 0;
	double delay_conf = 0; //confirmation delay
	::StatisticsData& operator+=(const StatisticsData& day);
};

struct EvalSegmentData {
	size_t drop_count = 0;

	double date = 0;
	double ref_time = -1;
	double detect_time = 0;
	double fp_time = -1;
	
	bool bDetected = false;
	bool bConfirmed = false;
	bool bFP = false;

	StatisticsData global;
	StatisticsData day;
};

class CEvaluation : public scgms::CBase_Filter {

protected:
	virtual HRESULT Do_Execute(scgms::UDevice_Event event) override final;
	virtual HRESULT Do_Configure(scgms::SFilter_Configuration configuration, refcnt::Swstr_list& error_description) override final;
public:
	CEvaluation(scgms::IFilter* output);
	virtual ~CEvaluation();

	virtual HRESULT IfaceCalling QueryInterface(const GUID* riid, void** ppvObj) override final;

private:
	GUID signal_ref;
	GUID signal_det;
	size_t th_drop = 36;
	int64_t max_delay = 180;
	int64_t fp_delay = 120;
	int64_t late_delay = 10;
	int64_t min_ref = 0;
	size_t th_detection = 1;
	size_t th_confirmation = 2;

	EvalSegmentData data;

	/*Process timed operations - late detection, reset timers*/
	void process_signal(scgms::UDevice_Event& event);
	/*Process reference signal - FP if not detected, set new reference time*/
	void process_reference(scgms::UDevice_Event& event);
	/*Process detected signal - TP/FP*/
	void process_detection(scgms::UDevice_Event& event);
};

#pragma warning( pop )
