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
#include <numeric>
//#include <>

#include "descriptor.h"
#include "swl.h"
#include "ML/ml.h"

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
    
    bool initialized = false;
    double prevL = -1;
    double prevT = -1;
    swl<double> ist;
    swl<double> activation_m;

    std::map<GUID, swl<double>> values;
    std::map<GUID, SFeatures> features;
};

/*Filter to detect physical activity*/
class CPa_Detection : public scgms::CBase_Filter {

protected:
	virtual HRESULT Do_Execute(scgms::UDevice_Event event) override final;
	virtual HRESULT Do_Configure(scgms::SFilter_Configuration configuration, refcnt::Swstr_list& error_description) override final;
public:
	CPa_Detection(scgms::IFilter* output);
	virtual ~CPa_Detection();

	virtual HRESULT IfaceCalling QueryInterface(const GUID* riid, void** ppvObj) override final;

private:
    std::map<uint64_t, PASegmentData> mSegments;

    std::vector<GUID> signals;
    std::map<GUID, double> th_signal;

    //GUID input_signal;
    //double th_signal = 0;

    //classifier use - test purposes only
    std::unique_ptr<ml> classifier;

    bool b_mean = false;
    size_t mean_window = 6;

    //edge detection
    GUID ist_signal;
    size_t ist_window = 12;
    double th_act = 2;
    double th_edge = -5.5;
    std::vector<double> thresholds = { 0.0125, 0.018 };
    std::vector<double> weights = { 2.25, 3 };

    /*Calc activation function*/
    double activation(scgms::UDevice_Event& event, PASegmentData& data);

    /*Calc features from the given vector*/
    SFeatures calc_features(std::vector<double>);
    /*Transform features to the vector*/
    std::vector<double> get_feature_vector(std::map<GUID, SFeatures> features);



    /*Calc mean value of vector*/
    template <typename T>
    inline T mean(std::vector<T> data)
    {
        T mean = std::accumulate(std::begin(data), std::end(data), 0.0) / data.size();
        return mean;
    }

    /*Calc median value of vector*/
    template <typename T>
    inline T median(std::vector<T> data)
    {
        T median = quantile<T>(data, { 0.5 }).front();
        return median;
    }

    /*Calc standard deviation of vector*/
    template <typename M>
    inline double std(std::vector<M>& data)
    {
        M mean = std::accumulate(std::begin(data), std::end(data), 0.0) / data.size();
        double sq_sum = std::inner_product(data.begin(), data.end(), data.begin(), 0.0);
        double stdev = std::sqrt(sq_sum / data.size() - (double)(mean * mean));
        return stdev;
    }
	
    template<typename T>
    static inline double Lerp(T v0, T v1, T t)
    {
        return (1 - t) * v0 + t * v1;
    }
	
    /*Calc quantiles of vector*/
    template<typename T>
    static inline std::vector<T> quantile(std::vector<T> data, const std::vector<T>& probs)
    {
        if (data.empty())
        {
            return std::vector<T>();
        }

        if (1 == data.size())
        {
            return std::vector<T>(1, data[0]);
        }

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
    inline T quantile_diff(std::vector<T> data)
    {
        std::vector<T> quantiles = quantile<T>(data, { 0.25, 0.75 });

        if (quantiles.size() < 2) return 0;

        T diff = quantiles[1] - quantiles[0];
        return diff;
    }
};

#pragma warning( pop )
