// $Id: channel.hpp 5188 2012-08-30 00:31:31Z dub $

/*
 Copyright (c) 2007-2012, Trustees of The Leland Stanford Junior University
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this 
 list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//////////////////////////////////////////////////////////////////////
//
//  File Name: channel.hpp
//
//  The Channel models a generic channel with a multi-cycle 
//   transmission delay. The channel latency can be specified as 
//   an integer number of simulator cycles.
//
/////

#include <queue>
#include <cassert>

#include "globals.hpp"
#include "module.hpp"
#include "timed_module.hpp"
#include "config_utils.hpp"
#include "channel.hpp"
#include "flit.hpp"
#include "credit.hpp"
#include "trafficmanager.hpp"

template class Channel<Flit>;
template class Channel<Credit>;

using namespace std;

template<typename T>
Channel<T>::Channel(const Configuration &config, Module * parent, string const & name)
  : TimedModule(parent, name), _delay(1), _input(0), _output(0) {
    _tm = config.tm;
}

template<typename T>
void Channel<T>::SetLatency(int cycles) {
  if(cycles <= 0) {
    Error("Channel must have positive delay.");
  }
  _delay = cycles ;
}

template<typename T>
void Channel<T>::Send(T * data) {
  _input = data;
}

template<typename T>
T * Channel<T>::Receive() {
  return _output;
}

template<typename T>
void Channel<T>::ReadInputs(bool chiplet_network) {
  if(_input) {
    _wait_queue.push(make_pair(_tm->getTime() + _delay - 1, _input));
    _input = 0;
  }
}

template<typename T>
void Channel<T>::WriteOutputs() {
  _output = 0;
  if(_wait_queue.empty()) {
    return;
  }
  pair<int, T *> const & item = _wait_queue.front();
  int const & time = item.first;
  if(_tm->getTime() < time) {
    return;
  }
  assert(_tm->getTime() == time);
  _output = item.second;
  assert(_output);
  _wait_queue.pop();
}
