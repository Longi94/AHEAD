// Copyright 2017 Till Kolditz
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*
 * memory.hpp
 *
 *  Created on: 11.10.2017
 *      Author: Till Kolditz - Till.Kolditz@gmail.com
 */

#pragma once

namespace ahead {

    template<size_t alignment, typename T>
    T*
    align_to(
            T * const pT) {
        size_t tmp = reinterpret_cast<size_t>(pT);
        return reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(pT) + (alignment - (tmp & (alignment - 1))));
    }

    template<typename T>
    bool inline is_power_of_two(
            T number) {
        return 0 == (number & (number - 1));
    }
}
