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

#include "descriptor.h"
#include "cho_detection.h"
#include "lstm_filter.h"
#include "savgol_filter.h"
#include "evaluation.h"
#include "pa_detection.h"

#include <iface/DeviceIface.h>
#include <iface/FilterIface.h>

#include <rtl/FilterLib.h>
#include <rtl/manufactory.h>
#include <utils/descriptor_utils.h>

#include <array>

/*
 * Example filter descriptor block
 */

namespace cho_detection {

	//CHO detection filter
	constexpr size_t cho_param_count = 9;

	const scgms::NParameter_Type cho_param_type[cho_param_count] = {
		scgms::NParameter_Type::ptSignal_Id,
		scgms::NParameter_Type::ptInt64,
		//scgms::NParameter_Type::ptWChar_Array,
		//scgms::NParameter_Type::ptWChar_Array,
		scgms::NParameter_Type::ptDouble_Array,
		scgms::NParameter_Type::ptBool,
		scgms::NParameter_Type::ptBool,
		scgms::NParameter_Type::ptDouble,
		scgms::NParameter_Type::ptBool,
		scgms::NParameter_Type::ptWChar_Array,
		scgms::NParameter_Type::ptDouble
	};

	const wchar_t* cho_ui_param_name[cho_param_count] = {
		L"Signal",
		L"Window size",
		//L"Activation thresholds",
		//L"Weights",
		L"Thresholds",
		L"Detect edges",
		L"Detect descending edges",
		L"Rise threshold",
		L"Use RNN",
		L"RNN model file path",
		L"RNN threshold"
	};

	const wchar_t* rsSignal = L"signal";
	const wchar_t* rsWindowSize = L"window_size";
	const wchar_t* rsThresholds = L"thresholds";
	const wchar_t* rsWeights = L"weights";
	const wchar_t* rsEdges = L"edges";
	const wchar_t* rsDesc = L"descending";
	const wchar_t* rsThAct = L"th_act";
	const wchar_t* rsRnn = L"rnn";
	const wchar_t* rsModelPath = L"model_path";
	const wchar_t* rsRnnThreshold = L"th_rnn";

	const wchar_t* cho_config_param_name[cho_param_count] = {
		rsSignal,
		rsWindowSize,
		rsThresholds,
		//rsWeights,
		rsEdges,
		rsDesc,
		rsThAct,
		rsRnn,
		rsModelPath,
		rsRnnThreshold
	};
	
	const scgms::TFilter_Descriptor cho_descriptor = {
		id_cho,
		scgms::NFilter_Flags::None,
		L"CHO detection",
		cho_param_count,
		cho_param_type,
		cho_ui_param_name,
		cho_config_param_name,
		nullptr
	};

	constexpr size_t cho_detect_thresh_and_weights_count = 4;
	const scgms::NModel_Parameter_Value cho_detect_thresh_and_weights_types[cho_detect_thresh_and_weights_count] = { scgms::NModel_Parameter_Value::mptDouble,scgms::NModel_Parameter_Value::mptDouble,scgms::NModel_Parameter_Value::mptDouble,scgms::NModel_Parameter_Value::mptDouble };
	const wchar_t* cho_detect_thresh_and_weights_ui_names[cho_detect_thresh_and_weights_count] = { L"Threshold Low", L"Weight Low", L"Threshold High", L"Weight High" };
	const wchar_t* cho_detect_thresh_and_weights_config_names[cho_detect_thresh_and_weights_count] = { L"Threshold Low", L"Weight Low", L"Threshold High", L"Weight High" };
	const double cho_detect_thresh_and_weights_lb[cho_detect_thresh_and_weights_count] = { 0.0, 0.0, 0.0, 0.0 };
	const double cho_detect_thresh_and_weights_def[cho_detect_thresh_and_weights_count] = { 0.0125, 2.25, 0.018, 3.0 };
	const double cho_detect_thresh_and_weights_ub[cho_detect_thresh_and_weights_count] = { 10.0, 10.0, 10.0, 10.0 };

	const scgms::TModel_Descriptor cho_detect_thresh_and_weights_desc = {
			id_cho,
			scgms::NModel_Flags::None,
			dsParameters,
			rsParameters,

			cho_detect_thresh_and_weights_count,
			cho_detect_thresh_and_weights_types,
			cho_detect_thresh_and_weights_ui_names,
			cho_detect_thresh_and_weights_config_names,

			cho_detect_thresh_and_weights_lb,
			cho_detect_thresh_and_weights_def,
			cho_detect_thresh_and_weights_ub,

			0,
			&scgms::signal_Null,
			&scgms::signal_Null
	};

	//RNN filter - deprecated
	const scgms::TFilter_Descriptor lstm_descriptor = {
		id_lstm,
		scgms::NFilter_Flags::None,
		L"LSTM",
		cho_param_count,
		cho_param_type,
		cho_ui_param_name,
		cho_config_param_name,
		nullptr
	};

	//Savitzky-Golay filter
	constexpr size_t savgol_param_count = 3;

	const scgms::NParameter_Type savgol_param_type[savgol_param_count] = {
		scgms::NParameter_Type::ptSignal_Id,
		scgms::NParameter_Type::ptInt64,
		scgms::NParameter_Type::ptInt64
	};

	const wchar_t* savgol_ui_param_name[savgol_param_count] = {
		L"Signal",
		L"Window size",
		L"Degree"
	};

	extern const wchar_t* rsSavgolSignal = L"savgol_signal";
	extern const wchar_t* rsSavgolWindow = L"savgol_window";
	extern const wchar_t* rsSavgolDeg = L"savgol_degree";

	const wchar_t* savgol_config_param_name[savgol_param_count] = {
		rsSavgolSignal,
		rsSavgolWindow,
		rsSavgolDeg
	};

	const scgms::TFilter_Descriptor savgol_descriptor = {
		id_savgol,
		scgms::NFilter_Flags::None,
		L"Savitzky-Golay filter",
		savgol_param_count,
		savgol_param_type,
		savgol_ui_param_name,
		savgol_config_param_name,
		nullptr
	};

	//evaluation filter
	constexpr size_t eval_param_count = 6;

	const scgms::NParameter_Type eval_param_type[eval_param_count] = {
		scgms::NParameter_Type::ptSignal_Id,
		scgms::NParameter_Type::ptSignal_Id,
		scgms::NParameter_Type::ptInt64,
		scgms::NParameter_Type::ptInt64,
		scgms::NParameter_Type::ptInt64,
		scgms::NParameter_Type::ptInt64
	};

	const wchar_t* eval_ui_param_name[eval_param_count] = {
		L"Reference signal",
		L"Detected signal",
		L"Max detection delay (min)",
		L"False positive cooldown",
		L"Late detection delay"
		L"Min reference count"
	};

	extern const wchar_t* rsSignalRef = L"ref_signal";
	extern const wchar_t* rsSignalDet = L"det_signal";
	extern const wchar_t* rsMaxDelay = L"max_delay";
	extern const wchar_t* rsFPDelay = L"fp_delay";
	extern const wchar_t* rsLateDelay = L"late_delay";
	extern const wchar_t* rsMinRef = L"min_ref";

	const wchar_t* eval_config_param_name[eval_param_count] = {
		rsSignalRef,
		rsSignalDet,
		rsMaxDelay,
		rsFPDelay,
		rsLateDelay,
		rsMinRef
	};
	
	const scgms::TFilter_Descriptor eval_descriptor = {
		id_eval,
		scgms::NFilter_Flags::None,
		L"Evaluation",
		eval_param_count,
		eval_param_type,
		eval_ui_param_name,
		eval_config_param_name,
		nullptr
	};

	//PA detection filter
	constexpr size_t pa_param_count = 14;

	const scgms::NParameter_Type pa_param_type[pa_param_count] = {
		scgms::NParameter_Type::ptBool,
		scgms::NParameter_Type::ptDouble,
		scgms::NParameter_Type::ptBool,
		scgms::NParameter_Type::ptDouble,
		scgms::NParameter_Type::ptBool,
		scgms::NParameter_Type::ptDouble,
		scgms::NParameter_Type::ptBool,
		scgms::NParameter_Type::ptDouble,

		scgms::NParameter_Type::ptBool,
		scgms::NParameter_Type::ptInt64,

		scgms::NParameter_Type::ptBool,
		scgms::NParameter_Type::ptSignal_Id,
		scgms::NParameter_Type::ptInt64,
		scgms::NParameter_Type::ptDouble_Array,
	};

	const wchar_t* pa_ui_param_name[pa_param_count] = {
		L"Heartbeat",
		L"Threshold",
		L"Steps",
		L"Threshold",
		L"Acceleration",
		L"Threshold",
		L"Electrodermal activity",
		L"Threshold",

		L"Mean",
		L"Size",

		L"Detect IST edges",
		L"Signal",
		L"Window size",
		L"Thresholds"
	};

	extern const wchar_t* rsSHeartbeat = L"b_heart";
	extern const wchar_t* rsSSteps = L"b_steps";
	extern const wchar_t* rsSAcc = L"b_acc";
	extern const wchar_t* rsSEl = L"b_el";
	extern const wchar_t* rsThHeartbeat = L"th_heart";
	extern const wchar_t* rsThSteps = L"th_steps";
	extern const wchar_t* rsThAcc = L"th_acc";
	extern const wchar_t* rsThEl = L"th_el";
	extern const wchar_t* rsMean = L"mean";
	extern const wchar_t* rsMeanSize = L"mean_size";

	const wchar_t* pa_config_param_name[pa_param_count] = {
		rsSHeartbeat,
		rsThHeartbeat,
		rsSSteps,
		rsThSteps,
		rsThAcc,
		rsSAcc,
		rsThEl,
		rsSEl,

		rsMean,
		rsMeanSize,

		rsDesc,
		rsSignal,
		rsWindowSize,
		rsThresholds
	};

	const scgms::TFilter_Descriptor pa_descriptor = {
		id_pa,
		scgms::NFilter_Flags::None,
		L"PA detection",
		pa_param_count,
		pa_param_type,
		pa_ui_param_name,
		pa_config_param_name,
		nullptr
	};

	const scgms::TModel_Descriptor pa_detect_thresh_and_weights_desc = {
			id_pa,
			scgms::NModel_Flags::None,
			dsParameters,
			rsParameters,

			cho_detect_thresh_and_weights_count,
			cho_detect_thresh_and_weights_types,
			cho_detect_thresh_and_weights_ui_names,
			cho_detect_thresh_and_weights_config_names,

			cho_detect_thresh_and_weights_lb,
			cho_detect_thresh_and_weights_def,
			cho_detect_thresh_and_weights_ub,

			0,
			&scgms::signal_Null,
			&scgms::signal_Null
	};

	//signal descriptors
	const scgms::TSignal_Descriptor activation_desc{ signal_activation, L"Activation", L"", scgms::NSignal_Unit::Other, 0xFFFF0000, 0xFFFF0000, scgms::NSignal_Visualization::smooth, scgms::NSignal_Mark::none, nullptr };
	const scgms::TSignal_Descriptor cho_desc{ signal_cho, L"CHO probability", L"", scgms::NSignal_Unit::Other, 0xFFFF0000, 0xFFFF0000, scgms::NSignal_Visualization::step, scgms::NSignal_Mark::cross, nullptr };
	const scgms::TSignal_Descriptor lstm_desc{ signal_lstm, L"LSTM", L"", scgms::NSignal_Unit::Other, 0xFFFF0000, 0xFFFF0000, scgms::NSignal_Visualization::smooth, scgms::NSignal_Mark::none, nullptr };
	const scgms::TSignal_Descriptor savgol_desc{ signal_savgol, L"Savgol signal", dsmmol_per_L, scgms::NSignal_Unit::mmol_per_L, 0xFFFF0000, 0xFFFF0000, scgms::NSignal_Visualization::smooth, scgms::NSignal_Mark::none, nullptr };
	const scgms::TSignal_Descriptor pa_desc{ signal_pa, L"PA detected", L"", scgms::NSignal_Unit::Other, 0xFFFF0000, 0xFFFF0000, scgms::NSignal_Visualization::step, scgms::NSignal_Mark::cross, nullptr };
}

/*
 * Array of available filter descriptors
 */

const std::array<scgms::TFilter_Descriptor, 5> filter_descriptions = { { cho_detection::cho_descriptor,
																		 cho_detection::lstm_descriptor,
																		 cho_detection::savgol_descriptor,
																		 cho_detection::eval_descriptor,
																		 cho_detection::pa_descriptor
																	 } };

const std::array<scgms::TSignal_Descriptor, 5> signal_descriptors = { { cho_detection::activation_desc,
																		cho_detection::lstm_desc,
																		cho_detection::savgol_desc,
																		cho_detection::cho_desc,
																		cho_detection::pa_desc
																	} };

const std::array<scgms::TModel_Descriptor, 2> model_descriptors = { { cho_detection::cho_detect_thresh_and_weights_desc,
																	  cho_detection::pa_detect_thresh_and_weights_desc
																  } };

/*
 * Filter library interface implementations
 */

extern "C" HRESULT IfaceCalling do_get_filter_descriptors(scgms::TFilter_Descriptor **begin, scgms::TFilter_Descriptor **end) {

	return do_get_descriptors(filter_descriptions, begin, end);
}

extern "C" HRESULT IfaceCalling do_get_signal_descriptors(scgms::TSignal_Descriptor * *begin, scgms::TSignal_Descriptor * *end) {

	return do_get_descriptors(signal_descriptors, begin, end);
}

extern "C" HRESULT IfaceCalling do_get_model_descriptors(scgms::TModel_Descriptor * *begin, scgms::TModel_Descriptor * *end) {
	//*begin = const_cast<scgms::TModel_Descriptor*>(&cho_detection::cho_detect_thresh_and_weights_desc);
	//*end = *begin + 1;
	//return S_OK;
	return do_get_descriptors(model_descriptors, begin, end);
}

extern "C" HRESULT IfaceCalling do_create_filter(const GUID *id, scgms::IFilter *output, scgms::IFilter **filter) {

	if (*id == cho_detection::cho_descriptor.id) {
		return Manufacture_Object<CCho_Detection>(filter, output);
	}

	if (*id == cho_detection::lstm_descriptor.id) {
		return Manufacture_Object<CLstm_filter>(filter, output);
	}

	if (*id == cho_detection::savgol_descriptor.id) {
		return Manufacture_Object<CSavgol_Filter>(filter, output);
	}

	if (*id == cho_detection::eval_descriptor.id) {
		return Manufacture_Object<CEvaluation>(filter, output);
	}

	if (*id == cho_detection::pa_descriptor.id) {
		return Manufacture_Object<CPa_Detection>(filter, output);
	}

	return E_NOTIMPL;
}
