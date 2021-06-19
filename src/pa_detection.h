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

#undef min
#undef max

#include <algorithm>
#include <cmath>
#include <vector>
#include <map>

#include "descriptor.h"
#include "swl.h"

#pragma warning( push )
#pragma warning( disable : 4250 ) // C4250 - 'class1' : inherits 'class2::member' via dominance

struct SFeatures {
    double mean = 0;
    double median = 0;
    double std = 0;
    double quantile = 0;
};

struct PASegmentData {
    double last_event_time = -1;

    std::map<GUID, swl<double>> values;
    std::map<GUID, SFeatures> features;
};

class CPa_Detection : public scgms::CBase_Filter {

protected:
	virtual HRESULT Do_Execute(scgms::UDevice_Event event) override final;
	virtual HRESULT Do_Configure(scgms::SFilter_Configuration configuration, refcnt::Swstr_list& error_description) override final;
public:
	CPa_Detection(scgms::IFilter* output);
	virtual ~CPa_Detection();

	virtual HRESULT IfaceCalling QueryInterface(const GUID* riid, void** ppvObj) override final;

private:
    /*std::map<uint64_t, swl<double>> mSegments;
	
	size_t window_size = 12;
	double th_acc = 0.5;
	
    size_t pa_detected_count = 0;*/

    std::map<uint64_t, PASegmentData> mSegments;

    size_t window_size = 12;
    bool bHeart = true;
    bool bSteps = true;
    bool bElectro = true;
    bool bTemp = true;
    bool bAcc = true;
    std::vector<GUID> signals;

    /*Calc features from the given vector*/
    SFeatures calc_features(std::vector<double>);


    /*Calc mean value of vector*/
    template <typename T>
    inline T mean(std::vector<T> Data)
    {
        T mean = std::accumulate(std::begin(Data), std::end(Data), 0.0) / Data.size();
        return mean;
    }

    /*Calc median value of vector*/
    template <typename T>
    inline T median(std::vector<T> Data)
    {
        T median = quantile<T>(Data, { 0.5 }).front();
        return median;
    }

    /*Calc standard deviation of vector*/
    template <typename M>
    inline double std(std::vector<M>& Data)
    {
        M mean = std::accumulate(std::begin(Data), std::end(Data), 0.0) / Data.size();
        double sq_sum = std::inner_product(Data.begin(), Data.end(), Data.begin(), 0.0);
        double stdev = std::sqrt(sq_sum / Data.size() - (double)(mean * mean));
        return stdev;
    }
	
    template<typename T>
    static inline double Lerp(T v0, T v1, T t)
    {
        return (1 - t) * v0 + t * v1;
    }
	
    /*Calc quantiles of vector*/
    template<typename T>
    static inline std::vector<T> quantile(swl<T>& inData, const std::vector<T>& probs)
    {
        if (inData.empty())
        {
            return std::vector<T>();
        }

        if (1 == inData.size())
        {
            return std::vector<T>(1, inData[0]);
        }

        std::vector<T> data = inData.to_vector();
        std::sort(data.begin(), data.end());
        std::vector<T> quantiles;

        for (size_t i = 0; i < probs.size(); ++i)
        {
            T poi = Lerp<T>(-0.5, data.size() - 0.5, probs[i]);

            size_t left = std::max(int64_t(std::floor(poi)), int64_t(0));
            size_t right = std::min(int64_t(std::ceil(poi)), int64_t(data.size() - 1));

            T datLeft = data.at(left);
            T datRight = data.at(right);

            T quantile = Lerp<T>(datLeft, datRight, poi - left);

            quantiles.push_back(quantile);
        }

        return quantiles;
    }

    /*Calc difference of 1. and 3. quantile*/
    template <typename T>
    inline T quantile_diff(std::vector<T> Data)
    {
        std::vector<T> quantiles = quantile<T>(data, { 0.25, 0.75 });
        T diff = quantiles[1] - quantiles[0];
        return diff;
    }
};

#pragma warning( pop )
