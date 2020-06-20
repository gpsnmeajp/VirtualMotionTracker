/*
MIT License

Copyright (c) 2020 gpsnmeajp

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <conio.h>
#include "SharedMemory.h"
#include "..\..\json.hpp"

using std::string;
using json = nlohmann::json;
int main(void)
{
	SharedMemory::SharedMemory sm;

	sm.open();
	while (!_kbhit()) {
		string r = sm.readM2D();
		if (!r.empty()) {
			json j = json::parse(r);
			string type = j["type"];

			printf("%s\n", type.c_str());
		}

		json jw;

		jw["type"] = "Hello";
		jw["json"] = json{ {"msg","Hello from cpp"} }.dump();
		sm.writeD2M(jw.dump());
		Sleep(800);
	}
	sm.close();

	return 0;
}

