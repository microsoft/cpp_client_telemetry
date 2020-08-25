///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2020 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MAT_LOGMANAGER_V1_HPP
#define MAT_LOGMANAGER_V1_HPP

// Example implementation of a 'classic' v1 LogManager API surface
#include "LogManagerBase.hpp"
namespace MAT_NS_BEGIN {
    class ModuleLogConfiguration : public ILogConfiguration {};
    class LogManager : public LogManagerBase<ModuleLogConfiguration> {};
} MAT_NS_END

#define LOGMANAGER_INSTANCE namespace MAT_NS_BEGIN { DEFINE_LOGMANAGER(LogManager, ModuleLogConfiguration); } MAT_NS_END

#endif
