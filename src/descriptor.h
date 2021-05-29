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

#include <rtl/guid.h>

namespace cho_detection {
	
	constexpr GUID id_cho = { 0xe4e55b55, 0x4ef, 0x4ed8, { 0x8c, 0x17, 0xf8, 0x78, 0x6e, 0x9b, 0x39, 0x3e } }; // {E4E55B55-04EF-4ED8-8C17-F8786E9B393E}
	constexpr GUID signal_activation = { 0x7cae17b7, 0x4887, 0x4874, { 0x95, 0xe3, 0x76, 0x6e, 0xfa, 0x7, 0x37, 0x7b } }; // {7CAE17B7-4887-4874-95E3-766EFA07377B}
	constexpr GUID signal_cho ={ 0x2db18d70, 0x5eb1, 0x4d91, { 0x8d, 0xfa, 0xe1, 0x65, 0xe1, 0xb0, 0xc4, 0x77 } }; // {2DB18D70-5EB1-4D91-8DFA-E165E1B0C477}

	extern const wchar_t* rsSignal;
	extern const wchar_t* rsWindowSize;
	extern const wchar_t* rsThresholds;
	extern const wchar_t* rsWeights;
	extern const wchar_t* rsThAct;
	extern const wchar_t* rsDesc;
	extern const wchar_t* rsRnn;
	extern const wchar_t* rsModelPath;
	extern const wchar_t* rsRnnThreshold;

	
	constexpr GUID id_lstm = { 0xb05c2db1, 0xb6a7, 0x45a6, { 0xb9, 0x73, 0xa6, 0x65, 0x7f, 0x83, 0xd7, 0x25 } }; // {B05C2DB1-B6A7-45A6-B973-A6657F83D725}
	constexpr GUID signal_lstm = { 0xb778a642, 0xc4c9, 0x473e, { 0x9c, 0x8c, 0xf1, 0x60, 0x30, 0x4f, 0x7, 0xc0 } }; // {B778A642-C4C9-473E-9C8C-F160304F07C0}

	
	constexpr GUID id_savgol = { 0xf45103c3, 0xe0e1, 0x4a8d, { 0xae, 0xc4, 0xb9, 0x7c, 0x83, 0x83, 0xf, 0x9f } }; // {F45103C3-E0E1-4A8D-AEC4-B97C83830F9F}
	constexpr GUID signal_savgol = { 0x94e92903, 0x10fb, 0x4e8b, { 0x95, 0xfc, 0xbe, 0x69, 0x16, 0xdd, 0x54, 0xa7 } }; // {94E92903-10FB-4E8B-95FC-BE6916DD54A7}

	extern const wchar_t* rsSavgolSignal;
	extern const wchar_t* rsSavgolWindow;
	extern const wchar_t* rsSavgolDeg;

	
	constexpr GUID id_eval = { 0xe4b7f5ba, 0xa3ba, 0x4f5e, { 0xb5, 0xa2, 0x3f, 0xee, 0x2b, 0x12, 0xde, 0xe5 } }; // {E4B7F5BA-A3BA-4F5E-B5A2-3FEE2B12DEE5}

	extern const wchar_t* rsSignalRef;
	extern const wchar_t* rsSignalDet;
	extern const wchar_t* rsMaxDelay;
	extern const wchar_t* rsFPDelay;
	extern const wchar_t* rsLateDelay;
	

	constexpr GUID id_pa = { 0xe9e99c04, 0xcb72, 0x4272, { 0xb6, 0xa, 0x3, 0x66, 0x47, 0x78, 0x3b, 0x6b } }; // {E9E99C04-CB72-4272-B60A-036647783B6B}
	constexpr GUID signal_pa = { 0xfeb4552a, 0x1c3, 0x4885, { 0x9b, 0xbc, 0x25, 0x29, 0x9b, 0xcd, 0xf0, 0x72 } }; // {FEB4552A-01C3-4885-9BBC-25299BCDF072}

	

}
