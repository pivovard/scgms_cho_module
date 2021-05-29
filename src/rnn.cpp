/*
 * @author = Bc. David Pivovar
 */

#include "rnn.h"

std::unique_ptr<fdeep::model> rnn::model;

void rnn::load_model(const wchar_t* path)
{
	std::wstring ws(path);
	std::string str(ws.begin(), ws.end());
	model = std::make_unique<fdeep::model>(fdeep::load_model(str));
}

void rnn::load_model(const std::string& path)
{
	model = std::make_unique<fdeep::model>(fdeep::load_model(path));
}

void rnn::load_model(const filesystem::path& path)
{
	std::string str(path.u8string());
	model = std::make_unique<fdeep::model>(fdeep::load_model(str));
}

float rnn::predict(scgms::UDevice_Event& event)
{
	c++;
	data.push_back(static_cast<float>(event.level()));
	if (data.size() > 1) {
		const float d = static_cast<float>(event.level() - *(data.end() - headers) / 5);
		data.push_back(d);
	}
	else {
		data.push_back(0.0f);
	}

	float date;
	float time = std::modf(event.device_time(), &date);
	float minute = time / scgms::One_Minute / 1440;
	float hour;
	std::modf(time / scgms::One_Hour, &hour);
	data.push_back(hour / 24);

	if (data.size() == window * headers) {
		const fdeep::tensor tensor = fdeep::tensor(fdeep::tensor_shape(window, headers), data.to_vector());
		return rnn::model->predict_single_output({ tensor });
	}

	return 0;
}
