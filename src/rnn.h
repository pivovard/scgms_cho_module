/*
 * @author = Bc. David Pivovar
 */

#pragma once

#undef min
#undef max

#include "fdeep/fdeep.hpp"
#include <rtl/DeviceLib.h>
#include <rtl/FilterLib.h>
#include "swl.h"

#include <string>

class rnn{
public:
	rnn() : data(24 * 3), window(24), headers(3) {};
	rnn(const size_t window, const size_t headers) : data(window * headers), window(window), headers(headers) {};

	static void load_model(const wchar_t* path);
	static void load_model(const std::string &path);
	static void load_model(const filesystem::path& path);
	float predict(scgms::UDevice_Event& event);

private:
	static std::unique_ptr<fdeep::model> model;
	swl<float> data;
	size_t window;
	size_t headers;
	int c = 0;
};
