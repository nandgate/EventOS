/******************************************************************************
* Copyright (C) <2017> NAND Gate Technologies LLC
* Permission is hereby granted, free of charge, to any person obtaining a copy 
* of this software and associated documentation files (the "Software"), to 
* deal in the Software without restriction, including without limitation the 
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
* sell copies of the Software, and to permit persons to whom the Software is 
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in 
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
* NAND GATE TECHNOLOGIES LLC BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
* THE SOFTWARE.
* 
* Except as contained in this notice, the name of NAND Gate Technologies LLC 
* shall not be used in advertising or otherwise to promote the sale, use or 
* other dealings in this Software without prior written authorization from 
* NAND Gate Technologies.
******************************************************************************/

#ifndef UNIT_TEST_H
#define UNIT_TEST_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// UnitTest
// Version: 0.0.1


// TODO: determine the _mockCallHistory array size. depth is the depth for a
// single function, this tracks all functions.  The upper bound is depth *
// number of mocks.

// Issue: How to reset the _mockCallIndex at the start of every test. Prefer
// somehow reset this auomagically?

// Note: Macros prefixed with '_' are private to the unit tester and are not
// intended to be used by test code directly.

#ifdef __cplusplus
    #define EXTERN_C extern "C" {
    #define END_EXTERN_C } 
#else
    #define EXTERN_C 
    #define END_EXTERN_C 
#endif

#define Mock_Vars(depth) \
    EXTERN_C \
    uint32_t _initAssertions; \
    uint32_t _assertions; \
    char *_note= NULL; \
    bool _mockExit= true; \
    enum { _mockDepth= depth } \
    END_EXTERN_C

#define _Mock_Exit() \
    if (_mockExit) { exit(1); }

#define _Mock_PrintFileLine() \
    if(_note != NULL) { \
        printf("\nAssert Note: %s", _note); \
    } \
    printf("\nLine: %d, File: %s\n", __LINE__, __FILE__) 

#define Assert_Note(msg) _note= msg;

#define Assert_ArrayEquals(len, expected, actual) ;{ \
    bool _fail= false; \
    for (uint32_t _i= 0; _i < len; _i++) { \
        if ((expected[_i]) != (actual[_i])) { \
            _Mock_PrintFileLine(); \
            printf("assert_ArrayEquals failed. Index: %d, Expected: 0x%08x, Actual: 0x%08x\n", _i, (uint32_t)(expected[_i]), (uint32_t)(actual[_i])); \
            _fail= true; \
            _Mock_Exit(); \
        } \
    } \
    if(!_fail) _assertions++; \
    }

#define Assert_Equals(expected, actual) \
    if ((expected) != (actual)) { \
        _Mock_PrintFileLine(); \
        printf("Assert_Equals failed. Expected: 0x%08x, Actual: 0x%08x\n", (uint32_t)(expected), (uint32_t)(actual)); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_EqualsLong(expected, actual) \
    if ((expected) != (actual)) { \
        _Mock_PrintFileLine(); \
        printf("Assert_EqualsLong failed. Expected: 0x%16lx, Actual: 0x%16lx\n", (expected), (actual)); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_EqualsFloat(expected, actual, delta) ;{ \
    float _d= (expected) - (actual); \
    if ((_d < 0.0f ? -_d : _d) > (delta)) { \
        _Mock_PrintFileLine(); \
        printf("Assert_EqualsFloat failed. Delta: %g, Expected: %g, Actual: %g\n", (delta), (expected), (actual)); \
        _Mock_Exit(); \
    }else { \
        _assertions++; \
    }}

#define Assert_Fail(msg) \
    _Mock_PrintFileLine(); \
    printf("Assert_Fail Message: %s\n", msg); \
    _Mock_Exit();

#define Assert_False(actual) \
    if (actual) { \
        _Mock_PrintFileLine(); \
        printf("Assert_False failed. Actual: %d\n", (actual)); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_NotEquals(expected, actual) \
    if ((expected) == (actual)) { \
        _Mock_PrintFileLine(); \
        printf("Assert_NotEquals failed. Expected: 0x%08x, Actual: 0x%08x\n", (uint32_t)(expected), (uint32_t)(actual)); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_IsNotNull(actual) \
    if (NULL == (actual)) { \
        _Mock_PrintFileLine(); \
        printf("Assert_IsNotNull failed. Actual: %p\n", (actual)); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_IsNull(actual) \
    if (NULL != (actual)) { \
        _Mock_PrintFileLine(); \
        printf("Assert_IsNull failed. Actual: %p\n", (actual)); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_StrEquals(expected, actual) ;{ \
    bool _fail= false; \
    uint32_t _elen = strlen(expected); \
    uint32_t _alen = strlen(actual); \
    if (_elen != _alen) { \
        _Mock_PrintFileLine(); \
        printf("Assert_StrEquals failed, lengths don't match. Expected len: %d, Actual len: %d\n", _elen, _alen); \
        _fail= true; \
        _Mock_Exit(); \
    } else { \
        for (uint32_t _i= 0; _i < _elen; _i++) { \
            if ((expected[_i]) != (actual[_i])) { \
                _Mock_PrintFileLine(); \
                printf("Assert_StrEquals failed. Index: %d, Expected: %c, Actual: %c\n", _i, (expected[_i]), (actual[_i])); \
                _fail= true; \
                _Mock_Exit(); \
            } \
        } \
    } \
    if(!_fail) _assertions++; \
    }

// TODO
#define Assert_StrContains(expected, actual) \
    Assert_Fail("TODO: Assert_StrContains()")

// TODO
#define Assert_SubString(pos, expected, actual) \
    Assert_Fail("TODO: Assert_SubString())")

#define Assert_True(actual) \
    if (!(actual)) { \
        _Mock_PrintFileLine(); \
        printf("Assert_True failed. Actual: %d\n", (actual)); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define _Mock_IncCallCount(fn) \
    if (_mock_##fn.callCount < _mockDepth) { \
        _mock_##fn.callCount++; \
    } else { \
        _Mock_PrintFileLine(); \
        printf("Mock_Depth failed: %s, depth: %d\n", #fn, _mockDepth); \
        _Mock_Exit() \
    }

#define Assert_Init() ;{ \
    FILE* _f= fopen("assertions.txt", "r"); \
    _initAssertions= 0; \
    if(_f == NULL) { \
        printf("Failed to open asserts.txt data file, resetting to zero.\n"); \
    } \
    else if(fscanf(_f, "%u", &_initAssertions) != 1) { \
        printf("Assert count not found in asserts.txt, resetting to zero.\n"); \
        fclose(_f); \
    } \
    _assertions= _initAssertions; \
    }

#define Assert_Save() ;{ \
    FILE* _f= fopen("assertions.txt", "w"); \
    if(_f == NULL) { \
        printf("Failed to open asserts.txt data file in Assert_Save()\n"); \
        _Mock_Exit(); \
    } \
    fprintf(_f, "%u\n", _assertions); \
    fclose(_f); \
    printf("Assertions: %d\n", (_assertions - _initAssertions)); \
    }

#define Mock_Void(fn) \
    EXTERN_C \
    struct { \
        uint32_t callCount; \
        void (*customMock)(void); \
    } _mock_##fn;\
    void fn(void) { \
        _Mock_IncCallCount(fn); \
        if(_mock_##fn.customMock) { \
            return _mock_##fn.customMock(); \
        } \
    } \
    END_EXTERN_C
    
#define Mock_Void1(fn, arg0_t) \
    EXTERN_C \
    struct { \
        uint32_t callCount; \
        void (*customMock)( arg0_t); \
         arg0_t arg0History[_mockDepth]; \
    } _mock_##fn; \
    void fn(arg0_t arg0) { \
        _mock_##fn.arg0History[_mock_##fn.callCount]= arg0; \
        _Mock_IncCallCount(fn); \
        if(_mock_##fn.customMock) { \
            return _mock_##fn.customMock(arg0); \
        } \
    } \
    END_EXTERN_C

#define Mock_Void2(fn, arg0_t, arg1_t) \
    EXTERN_C \
    struct { \
        uint32_t callCount; \
        void (*customMock)(arg0_t, arg1_t); \
        arg0_t arg0History[_mockDepth]; \
        arg1_t arg1History[_mockDepth]; \
    } _mock_##fn; \
    void fn(arg0_t arg0, arg1_t arg1) { \
        _mock_##fn.arg0History[_mock_##fn.callCount]= arg0; \
        _mock_##fn.arg1History[_mock_##fn.callCount]= arg1; \
        _Mock_IncCallCount(fn); \
        if(_mock_##fn.customMock) { \
            return _mock_##fn.customMock(arg0, arg1); \
        } \
    } \
    END_EXTERN_C

#define Mock_Void3(fn, arg0_t, arg1_t, arg2_t) \
    EXTERN_C \
    struct { \
        uint32_t callCount; \
        void (*customMock)(arg0_t, arg1_t, arg2_t); \
        arg0_t arg0History[_mockDepth]; \
        arg1_t arg1History[_mockDepth]; \
        arg2_t arg2History[_mockDepth]; \
    } _mock_##fn; \
    void fn(arg0_t arg0, arg1_t arg1, arg2_t arg2) { \
        _mock_##fn.arg0History[_mock_##fn.callCount]= arg0; \
        _mock_##fn.arg1History[_mock_##fn.callCount]= arg1; \
        _mock_##fn.arg2History[_mock_##fn.callCount]= arg2; \
        _Mock_IncCallCount(fn); \
        if(_mock_##fn.customMock) { \
            return _mock_##fn.customMock(arg0, arg1, arg2); \
        } \
    } \
    END_EXTERN_C

#define Mock_Void4(fn, arg0_t, arg1_t, arg2_t, arg3_t) \
    EXTERN_C \
    struct { \
        uint32_t callCount; \
        void (*customMock)(arg0_t, arg1_t, arg2_t, arg3_t); \
        arg0_t arg0History[_mockDepth]; \
        arg1_t arg1History[_mockDepth]; \
        arg2_t arg2History[_mockDepth]; \
        arg3_t arg3History[_mockDepth]; \
    } _mock_##fn; \
    void fn(arg0_t arg0, arg1_t arg1, arg2_t arg2, arg3_t arg3) { \
        _mock_##fn.arg0History[_mock_##fn.callCount]= arg0; \
        _mock_##fn.arg1History[_mock_##fn.callCount]= arg1; \
        _mock_##fn.arg2History[_mock_##fn.callCount]= arg2; \
        _mock_##fn.arg3History[_mock_##fn.callCount]= arg3; \
        _Mock_IncCallCount(fn); \
        if(_mock_##fn.customMock) { \
            return _mock_##fn.customMock(arg0, arg1, arg2, arg3); \
        } \
    } \
    END_EXTERN_C

#define Mock_Void5(fn, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t) \
    EXTERN_C \
    struct { \
        uint32_t callCount; \
        void (*customMock)(arg0_t, arg1_t, arg2_t, arg3_t, arg4_t); \
        arg0_t arg0History[_mockDepth]; \
        arg1_t arg1History[_mockDepth]; \
        arg2_t arg2History[_mockDepth]; \
        arg3_t arg3History[_mockDepth]; \
        arg3_t arg4History[_mockDepth]; \
    } _mock_##fn; \
    void fn(arg0_t arg0, arg1_t arg1, arg2_t arg2, arg3_t arg3, arg4_t arg4) { \
        _mock_##fn.arg0History[_mock_##fn.callCount]= arg0; \
        _mock_##fn.arg1History[_mock_##fn.callCount]= arg1; \
        _mock_##fn.arg2History[_mock_##fn.callCount]= arg2; \
        _mock_##fn.arg3History[_mock_##fn.callCount]= arg3; \
        _mock_##fn.arg4History[_mock_##fn.callCount]= arg4; \
        _Mock_IncCallCount(fn); \
        if(_mock_##fn.customMock) { \
            return _mock_##fn.customMock(arg0, arg1, arg2, arg3, arg4); \
        } \
    } \
    END_EXTERN_C

#define _Mock_Return(fn) \
    if(!_mock_##fn.isReturnSet) { \
        _Mock_PrintFileLine(); \
        printf("Mock Fail: return data not set for %s\n", #fn); \
        _Mock_Exit(); \
    } else if(_mock_##fn.sequenceLength) { \
        if(_mock_##fn.sequenceIdx < _mock_##fn.sequenceLength) { \
            _mock_##fn.retHistory[_mock_##fn.callCount-1]= _mock_##fn.sequence[_mock_##fn.sequenceIdx++]; \
            return  _mock_##fn.retHistory[_mock_##fn.callCount-1]; \
        } \
        _mock_##fn.retHistory[_mock_##fn.callCount-1]= _mock_##fn.sequence[_mock_##fn.sequenceIdx-1]; \
        return  _mock_##fn.retHistory[_mock_##fn.callCount-1]; \
    } \
    _mock_##fn.retHistory[_mock_##fn.callCount-1]= _mock_##fn.returnValue; \
    return  _mock_##fn.retHistory[_mock_##fn.callCount-1]

#define Mock_Reset(fn) \
    memset(&_mock_##fn, 0, sizeof(_mock_##fn))

#define Mock_Custom(fn, custom) \
    _mock_##fn.customMock= custom

#define Mock_Returns(fn,retValue) \
    _mock_##fn.returnValue= retValue; \
    _mock_##fn.isReturnSet= true

#define Mock_ReturnsSequence(fn, len, seq) \
    _mock_##fn.sequenceLength= len; \
    _mock_##fn.sequence= seq; \
    _mock_##fn.isReturnSet= true

#define Mock_Value(ret_t, fn) \
    EXTERN_C \
    struct { \
        uint32_t callCount; \
        ret_t (*customMock)(void); \
        bool isReturnSet; \
        ret_t retHistory[_mockDepth]; \
        ret_t returnValue; \
        uint32_t sequenceLength; \
        uint32_t sequenceIdx; \
        ret_t *sequence; \
    } _mock_##fn; \
    ret_t fn(void) { \
        _Mock_IncCallCount(fn); \
        if(_mock_##fn.customMock) { \
            _mock_##fn.retHistory[_mock_##fn.callCount-1]= _mock_##fn.customMock(); \
            return  _mock_##fn.retHistory[_mock_##fn.callCount-1]; \
        } \
        else { _Mock_Return(fn); }\
    }
    END_EXTERN_C

#define Mock_Value1(ret_t, fn, arg0_t) \
    EXTERN_C \
    struct { \
        uint32_t callCount; \
        ret_t (*customMock)(arg0_t); \
        arg0_t arg0History[_mockDepth]; \
        bool isReturnSet; \
        ret_t retHistory[_mockDepth]; \
        ret_t returnValue; \
        uint32_t sequenceLength; \
        uint32_t sequenceIdx; \
        ret_t *sequence; \
    } _mock_##fn; \
    ret_t fn(arg0_t arg0) { \
        _mock_##fn.arg0History[_mock_##fn.callCount]= arg0; \
        _Mock_IncCallCount(fn); \
        if(_mock_##fn.customMock) { \
            _mock_##fn.retHistory[_mock_##fn.callCount-1]= _mock_##fn.customMock(arg0); \
            return  _mock_##fn.retHistory[_mock_##fn.callCount-1]; \
        } \
        else { _Mock_Return(fn); } \
    } \
    END_EXTERN_C


#define Mock_Value2(ret_t, fn, arg0_t, arg1_t) \
    EXTERN_C \
    struct { \
        uint32_t callCount; \
        ret_t (*customMock)(arg0_t, arg1_t); \
        arg0_t arg0History[_mockDepth]; \
        arg1_t arg1History[_mockDepth]; \
        ret_t retHistory[_mockDepth]; \
        bool isReturnSet; \
        ret_t returnValue; \
        uint32_t sequenceLength; \
        uint32_t sequenceIdx; \
        ret_t *sequence; \
    } _mock_##fn; \
    ret_t fn(arg0_t arg0, arg1_t arg1) { \
        _mock_##fn.arg0History[_mock_##fn.callCount]= arg0; \
        _mock_##fn.arg1History[_mock_##fn.callCount]= arg1; \
        _Mock_IncCallCount(fn); \
        if(_mock_##fn.customMock) { \
            _mock_##fn.retHistory[_mock_##fn.callCount-1]= _mock_##fn.customMock(arg0, arg1); \
            return  _mock_##fn.retHistory[_mock_##fn.callCount-1]; \
        } \
        else { _Mock_Return(fn); } \
    } \
    END_EXTERN_C

#define Mock_Value3(ret_t, fn, arg0_t, arg1_t, arg2_t) \
    EXTERN_C \
    struct { \
        uint32_t callCount; \
        ret_t (*customMock)(arg0_t, arg1_t, arg2_t); \
        arg0_t arg0History[_mockDepth]; \
        arg1_t arg1History[_mockDepth]; \
        arg2_t arg2History[_mockDepth]; \
        ret_t retHistory[_mockDepth]; \
        bool isReturnSet; \
        ret_t returnValue; \
        uint32_t sequenceLength; \
        uint32_t sequenceIdx; \
        ret_t *sequence; \
    } _mock_##fn; \
    ret_t fn(arg0_t arg0, arg1_t arg1, arg2_t arg2) { \
        _mock_##fn.arg0History[_mock_##fn.callCount]= arg0; \
        _mock_##fn.arg1History[_mock_##fn.callCount]= arg1; \
        _mock_##fn.arg2History[_mock_##fn.callCount]= arg2; \
        _Mock_IncCallCount(fn); \
        if(_mock_##fn.customMock) { \
            _mock_##fn.retHistory[_mock_##fn.callCount-1]= _mock_##fn.customMock(arg0, arg1, arg2); \
            return  _mock_##fn.retHistory[_mock_##fn.callCount-1]; \
        } \
        else { _Mock_Return(fn); } \
    }
    END_EXTERN_C

#define Mock_Value4(ret_t, fn, arg0_t, arg1_t, arg2_t, arg3_t) \
    EXTERN_C \
    struct { \
        uint32_t callCount; \
        ret_t (*customMock)(arg0_t, arg1_t, arg2_t, arg3_t); \
        arg0_t arg0History[_mockDepth]; \
        arg1_t arg1History[_mockDepth]; \
        arg2_t arg2History[_mockDepth]; \
        arg3_t arg3History[_mockDepth]; \
        ret_t retHistory[_mockDepth]; \
        bool isReturnSet; \
        ret_t returnValue; \
        uint32_t sequenceLength; \
        uint32_t sequenceIdx; \
        ret_t *sequence; \
    } _mock_##fn; \
    ret_t fn(arg0_t arg0, arg1_t arg1, arg2_t arg2, arg3_t arg3) { \
        _mock_##fn.arg0History[_mock_##fn.callCount]= arg0; \
        _mock_##fn.arg1History[_mock_##fn.callCount]= arg1; \
        _mock_##fn.arg2History[_mock_##fn.callCount]= arg2; \
        _mock_##fn.arg3History[_mock_##fn.callCount]= arg3; \
        _Mock_IncCallCount(fn); \
        if(_mock_##fn.customMock) { \
            _mock_##fn.retHistory[_mock_##fn.callCount-1]= _mock_##fn.customMock(arg0, arg1, arg2, arg3); \
            return  _mock_##fn.retHistory[_mock_##fn.callCount-1]; \
        } \
        else { _Mock_Return(fn); } \
    } \
    END_EXTERN_C

#define Mock_Value5(ret_t, fn, arg0_t, arg1_t, arg2_t, arg3_t, arg4_t) \
    EXTERN_C \
    struct { \
        uint32_t callCount; \
        ret_t (*customMock)(arg0_t, arg1_t, arg2_t, arg3_t, arg4_t); \
        arg0_t arg0History[_mockDepth]; \
        arg1_t arg1History[_mockDepth]; \
        arg2_t arg2History[_mockDepth]; \
        arg3_t arg3History[_mockDepth]; \
        arg4_t arg4History[_mockDepth]; \
        ret_t retHistory[_mockDepth]; \
        bool isReturnSet; \
        ret_t returnValue; \
        uint32_t sequenceLength; \
        uint32_t sequenceIdx; \
        ret_t *sequence; \
    } _mock_##fn; \
    ret_t fn(arg0_t arg0, arg1_t arg1, arg2_t arg2, arg3_t arg3, arg4_t arg4) { \
        _mock_##fn.arg0History[_mock_##fn.callCount]= arg0; \
        _mock_##fn.arg1History[_mock_##fn.callCount]= arg1; \
        _mock_##fn.arg2History[_mock_##fn.callCount]= arg2; \
        _mock_##fn.arg3History[_mock_##fn.callCount]= arg3; \
        _mock_##fn.arg4History[_mock_##fn.callCount]= arg4; \
        _Mock_IncCallCount(fn); \
        if(_mock_##fn.customMock) { \
            _mock_##fn.retHistory[_mock_##fn.callCount-1]= _mock_##fn.customMock(arg0, arg1, arg2, arg3, arg4); \
            return  _mock_##fn.retHistory[_mock_##fn.callCount-1]; \
        } \
        else { _Mock_Return(fn); } \
    } \
    END_EXTERN_C

#define Assert_Returned(fn, expected) ;{ \
    bool _match= false; \
    for (uint32_t _i= 0; _i < _mock_##fn.callCount; _i++) { \
        _match |= (_mock_##fn.retHistory[_i] == (expected)); \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_Returned failed: %s, Calls: %d, Expected: %d\n", #fn, _mock_##fn.callCount, (expected)); \
        for (uint32_t _i= 0; _i < _mock_##fn.callCount; _i++) { \
            printf("Call: %d, Actual: %d\n", _i, _mock_##fn.retHistory[_i]); \
        } \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} 

#define Assert_CallCount(count, fn) \
    if(_mock_##fn.callCount != count) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CallCount failed: %s, Expected: %d, Actual: %d\n", #fn, count, _mock_##fn.callCount); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledOnce(fn) \
    if(_mock_##fn.callCount != 1) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledOnce failed: %s, Actual: %d\n", #fn, _mock_##fn.callCount); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_NotCalled(fn) \
    if(_mock_##fn.callCount != 0) { \
        _Mock_PrintFileLine(); \
        printf("Assert_NotCalled failed: %s, Actual: %d\n", #fn, _mock_##fn.callCount); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }


#define Assert_CallOrder(fn1st, fn2nd) \
    Assert_Fail("TODO: Assert_CallOrder()")

#define Assert_Called0(fn) ;{\
    if(_mock_##fn.callCount == 0) { \
        _Mock_PrintFileLine(); \
        printf("Assert_Called failed: %s, Calls: %d\n", #fn, _mock_##fn.callCount); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} 

#define Assert_Called1(fn, arg0) ;{\
    bool _match= false; \
    for (uint32_t _i= 0; _i < _mock_##fn.callCount; _i++) { \
        _match |= (_mock_##fn.arg0History[_i] == (arg0)); \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_Called1 failed: %s, Calls: %d, Expected: %d\n", #fn, _mock_##fn.callCount, (uint32_t)(arg0)); \
        for (uint32_t _i= 0; _i < _mock_##fn.callCount; _i++) { \
            printf("Call: %d, Actual: %d\n", _i, (uint32_t)_mock_##fn.arg0History[_i]); \
        } \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} 

#define Assert_Called2(fn, arg0, arg1) ;{\
    bool _match= false; \
    for (uint32_t _i= 0; _i < _mock_##fn.callCount; _i++) { \
        _match |= (_mock_##fn.arg0History[_i] == (arg0)) & \
                  (_mock_##fn.arg1History[_i] == (arg1)); \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_Called2 failed: %s, Calls: %d, Expected: %d, %d\n", #fn, _mock_##fn.callCount, (uint32_t)(arg0), (uint32_t)(arg1)); \
        for (uint32_t _i= 0; _i < _mock_##fn.callCount; _i++) { \
            printf("Call: %d, Actual: %d, %d\n", _i, (uint32_t)_mock_##fn.arg0History[_i], (uint32_t)_mock_##fn.arg1History[_i]); \
        } \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_Called3(fn, arg0, arg1, arg2) ;{\
    bool _match= false; \
    for (uint32_t _i= 0; _i < _mock_##fn.callCount; _i++) { \
        _match |= (_mock_##fn.arg0History[_i] == (arg0)) & \
                  (_mock_##fn.arg1History[_i] == (arg1)) & \
                  (_mock_##fn.arg2History[_i] == (arg2)); \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_Called3 failed: %s, Calls: %d, Expected: %d, %d, %d\n", #fn, _mock_##fn.callCount, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)(arg2)); \
        for (uint32_t _i= 0; _i < _mock_##fn.callCount; _i++) { \
            printf("Call: %d, Actual: %d, %d, %d\n", _i, (uint32_t)_mock_##fn.arg0History[_i],(uint32_t) _mock_##fn.arg1History[_i], (uint32_t)_mock_##fn.arg2History[_i]); \
        } \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_Called4(fn, arg0, arg1, arg2, arg3) ;{\
    bool _match= false; \
    for (uint32_t _i= 0; _i < _mock_##fn.callCount; _i++) { \
        _match |= (_mock_##fn.arg0History[_i] == (arg0)) & \
                  (_mock_##fn.arg1History[_i] == (arg1)) & \
                  (_mock_##fn.arg2History[_i] == (arg2)) & \
                  (_mock_##fn.arg3History[_i] == (arg3)); \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_Called4 failed: %s, Calls: %d, Expected: %d, %d, %d, %d\n", #fn, _mock_##fn.callCount, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)(arg2), (uint32_t)(arg3)); \
        for (uint32_t _i= 0; _i < _mock_##fn.callCount; _i++) { \
            printf("Call: %d, Actual: %d, %d, %d, %d\n", _i, (uint32_t)_mock_##fn.arg0History[_i], (uint32_t)_mock_##fn.arg1History[_i], (uint32_t)_mock_##fn.arg2History[_i], (uint32_t)_mock_##fn.arg3History[_i]); \
        } \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_Called5(fn, arg0, arg1, arg2, arg3, arg4) ;{\
    bool _match= false; \
    for (uint32_t _i= 0; _i < _mock_##fn.callCount; _i++) { \
        _match |= (_mock_##fn.arg0History[_i] == (arg0)) & \
                  (_mock_##fn.arg1History[_i] == (arg1)) & \
                  (_mock_##fn.arg2History[_i] == (arg2)) & \
                  (_mock_##fn.arg3History[_i] == (arg3)) & \
                  (_mock_##fn.arg4History[_i] == (arg4)); \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_Called5 failed: %s, Calls: %d, Expected: %d, %d, %d, %d, %d\n", #fn, _mock_##fn.callCount, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)(arg2), (uint32_t)(arg3), (uint32_t)(arg4)); \
        for (uint32_t _i= 0; _i < _mock_##fn.callCount; _i++) { \
            printf("Call: %d, Actual: %d, %d, %d, %d, %d\n", _i, (uint32_t)_mock_##fn.arg0History[_i], (uint32_t)_mock_##fn.arg1History[_i], (uint32_t)_mock_##fn.arg2History[_i], (uint32_t)_mock_##fn.arg3History[_i], (uint32_t)_mock_##fn.arg4History[_i]); \
        } \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_CalledFirst1(fn, arg0) \
    if(_mock_##fn.callCount == 0) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledFirst1 failed: %s, Not called\n", #fn); \
        _Mock_Exit(); \
    } else if(_mock_##fn.arg0History[0] != (arg0)) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledFirst1 failed: %s, Expected: %d, Actual: %d\n", #fn, (uint32_t)(arg0), (uint32_t)_mock_##fn.arg0History[0]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledFirst2(fn, arg0, arg1) \
    if(_mock_##fn.callCount == 0) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledFirst2 failed: %s, Not called\n", #fn); \
        _Mock_Exit(); \
    } else if((_mock_##fn.arg0History[0] != (arg0)) || \
              (_mock_##fn.arg1History[0] != (arg1))) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledFirst2 failed: %s, Expected: %d, %d, Actual: %d, %d\n", #fn, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)_mock_##fn.arg0History[0], (uint32_t)_mock_##fn.arg1History[1]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledFirst3(fn, arg0, arg1, arg2) \
    if(_mock_##fn.callCount == 0) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledFirst3 failed: %s, Not called\n", #fn); \
        _Mock_Exit(); \
    } else if((_mock_##fn.arg0History[0] != (arg0)) || \
              (_mock_##fn.arg1History[0] != (arg1)) || \
              (_mock_##fn.arg2History[0] != (arg2))) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledFirst3 failed: %s, Expected: %d, %d, %d, Actual: %d, %d, %d\n", #fn, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)(arg2), (uint32_t)_mock_##fn.arg0History[0], (uint32_t)_mock_##fn.arg2History[1], (uint32_t)_mock_##fn.arg2History[1]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledFirst4(fn, arg0, arg1, arg2, arg3) \
    if(_mock_##fn.callCount == 0) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledFirst4 failed: %s, Not called\n", #fn); \
        _Mock_Exit(); \
    } else if((_mock_##fn.arg0History[0] != (arg0)) || \
              (_mock_##fn.arg1History[0] != (arg1)) || \
              (_mock_##fn.arg2History[0] != (arg2)) || \
              (_mock_##fn.arg3History[0] != (arg3))) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledFirst4 failed: %s, Expected: %d, %d, %d, %d Actual: %d, %d, %d, %d\n", #fn, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)(arg2), (uint32_t)(arg3), (uint32_t)_mock_##fn.arg0History[0], (uint32_t)_mock_##fn.arg1History[1], (uint32_t)_mock_##fn.arg2History[1], (uint32_t)_mock_##fn.arg3History[1]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledFirst5(fn, arg0, arg1, arg2, arg3, arg4) \
    if(_mock_##fn.callCount == 0) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledFirst5 failed: %s, Not called\n", #fn); \
        _Mock_Exit(); \
    } else if((_mock_##fn.arg0History[0] != (arg0)) || \
              (_mock_##fn.arg1History[0] != (arg1)) || \
              (_mock_##fn.arg2History[0] != (arg2)) || \
              (_mock_##fn.arg3History[0] != (arg3)) || \
              (_mock_##fn.arg4History[0] != (arg4))) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledFirst4 failed: %s, Expected: %d, %d, %d, %d, %d Actual: %d, %d, %d, %d, %d\n", #fn, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)(arg2), (uint32_t)(arg3), (uint32_t)(arg4), (uint32_t)_mock_##fn.arg0History[0], (uint32_t)_mock_##fn.arg1History[1], (uint32_t)_mock_##fn.arg2History[1], (uint32_t)_mock_##fn.arg3History[1], (uint32_t)_mock_##fn.arg4History[1]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledLast1(fn, arg0) \
    if(_mock_##fn.callCount == 0) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledLast1 failed: %s, Not called\n", #fn); \
        _Mock_Exit(); \
    } else if(_mock_##fn.arg0History[_mock_##fn.callCount-1] != (arg0)) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledLast1 failed: %s, Expected: %d, Actual: %d\n", #fn,(uint32_t)(arg0), (uint32_t) _mock_##fn.arg0History[_mock_##fn.callCount-1]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledLast2(fn, arg0, arg1) \
    if(_mock_##fn.callCount == 0) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledLast2 failed: %s, Not called\n", #fn); \
        _Mock_Exit(); \
    } else if((_mock_##fn.arg0History[_mock_##fn.callCount-1] != (arg0)) || \
              (_mock_##fn.arg1History[_mock_##fn.callCount-1] != (arg1))) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledLast2 failed: %s, Expected: %d, %d, Actual: %d, %d\n", #fn, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)_mock_##fn.arg0History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg1History[_mock_##fn.callCount-1]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledLast3(fn, arg0, arg1, arg2) \
    if(_mock_##fn.callCount == 0) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledLast3 failed: %s, Not called\n", #fn); \
        _Mock_Exit(); \
    } else if((_mock_##fn.arg0History[_mock_##fn.callCount-1] != (arg0)) || \
              (_mock_##fn.arg1History[_mock_##fn.callCount-1] != (arg1)) || \
              (_mock_##fn.arg2History[_mock_##fn.callCount-1] != (arg2))) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledLast3 failed: %s, Expected: %d, %d, %d Actual: %d, %d, %d\n", #fn, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)(arg2), (uint32_t)_mock_##fn.arg0History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg1History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg2History[_mock_##fn.callCount-1]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledLast4(fn, arg0, arg1, arg2, arg3) \
    if(_mock_##fn.callCount == 0) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledLast4 failed: %s, Not called\n", #fn); \
        _Mock_Exit(); \
    } else if((_mock_##fn.arg0History[_mock_##fn.callCount-1] != (arg0)) || \
              (_mock_##fn.arg1History[_mock_##fn.callCount-1] != (arg1)) || \
              (_mock_##fn.arg2History[_mock_##fn.callCount-1] != (arg2)) || \
              (_mock_##fn.arg3History[_mock_##fn.callCount-1] != (arg3))) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledLast4 failed: %s, Expected: %d, %d, %d, %d Actual: %d, %d, %d, %d\n", #fn, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)(uint32_t)(arg2), (uint32_t)(arg3), (uint32_t)_mock_##fn.arg0History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg1History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg2History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg3History[_mock_##fn.callCount-1]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledLast5(fn, arg0, arg1, arg2, arg3, arg4) \
    if(_mock_##fn.callCount == 0) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledLast5 failed: %s, Not called\n", #fn); \
        _Mock_Exit(); \
    } else if((_mock_##fn.arg0History[_mock_##fn.callCount-1] != (arg0)) || \
              (_mock_##fn.arg1History[_mock_##fn.callCount-1] != (arg1)) || \
              (_mock_##fn.arg2History[_mock_##fn.callCount-1] != (arg2)) || \
              (_mock_##fn.arg3History[_mock_##fn.callCount-1] != (arg3)) || \
              (_mock_##fn.arg4History[_mock_##fn.callCount-1] != (arg4))) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledLast5 failed: %s, Expected: %d, %d, %d, %d, %d Actual: %d, %d, %d, %d, %d\n", #fn, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)(arg2), (uint32_t)(arg3), (uint32_t)(arg4), (uint32_t)_mock_##fn.arg0History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg1History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg2History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg3History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg4History[_mock_##fn.callCount-1]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledN1(n, fn, arg0) \
    if(_mock_##fn.callCount < n) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledN1 failed: %s, Not called or called too few times, called: %d\n", #fn, _mock_##fn.callCount); \
        _Mock_Exit(); \
    } else if(_mock_##fn.arg0History[n-1] != (arg0)) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledN1 failed: %s, Expected: %d, Actual: %d\n", #fn, (uint32_t)(arg0), (uint32_t)_mock_##fn.arg0History[_mock_##fn.callCount-1]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledN2(n, fn, arg0, arg1) \
    if(_mock_##fn.callCount < n) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledN2 failed: %s, Not called or called too few times, called: %d\n", #fn, _mock_##fn.callCount); \
        _Mock_Exit(); \
    } else if((_mock_##fn.arg0History[n-1] != (arg0)) || \
              (_mock_##fn.arg1History[n-1] != (arg1))) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledN2 failed: %s, Expected: %d, %d, Actual: %d, %d\n", #fn, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)_mock_##fn.arg0History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg1History[_mock_##fn.callCount-1]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledN3(n, fn, arg0, arg1, arg2) \
    if(_mock_##fn.callCount < n) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledN3 failed: %s, Not called or called too few times, called: %d\n", #fn, _mock_##fn.callCount); \
        _Mock_Exit(); \
    } else if((_mock_##fn.arg0History[n-1] != (arg0)) || \
              (_mock_##fn.arg1History[n-1] != (arg1)) || \
              (_mock_##fn.arg2History[n-1] != (arg2))) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledN3 failed: %s, Expected: %d, %d, %d, Actual: %d, %d, %d\n", #fn, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)(arg2), (uint32_t)_mock_##fn.arg0History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg1History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg2History[_mock_##fn.callCount-1]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledN4(n, fn, arg0, arg1, arg2, arg3) \
    if(_mock_##fn.callCount < n) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledN4 failed: %s, Not called or called too few times, called: %d\n", #fn, _mock_##fn.callCount); \
        _Mock_Exit(); \
    } else if((_mock_##fn.arg0History[n-1] != (arg0)) || \
              (_mock_##fn.arg1History[n-1] != (arg1)) || \
              (_mock_##fn.arg2History[n-1] != (arg2)) || \
              (_mock_##fn.arg3History[n-1] != (arg3))) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledN4 failed: %s, Expected: %d, %d, %d, %d Actual: %d, %d, %d, %d\n", #fn, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)(arg2), (uint32_t)(arg3), (uint32_t)_mock_##fn.arg0History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg1History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg2History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg3History[_mock_##fn.callCount-1]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_CalledN5(n, fn, arg0, arg1, arg2, arg3, arg4) \
    if(_mock_##fn.callCount < n) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledN5 failed: %s, Not called or called too few times, called: %d\n", #fn, _mock_##fn.callCount); \
        _Mock_Exit(); \
    } else if((_mock_##fn.arg0History[n-1] != (arg0)) || \
              (_mock_##fn.arg1History[n-1] != (arg1)) || \
              (_mock_##fn.arg2History[n-1] != (arg2)) || \
              (_mock_##fn.arg3History[n-1] != (arg3)) || \
              (_mock_##fn.arg4History[n-1] != (arg4))) { \
        _Mock_PrintFileLine(); \
        printf("Assert_CalledN4 failed: %s, Expected: %d, %d, %d, %d, %d Actual: %d, %d, %d, %d, %d\n", #fn, (uint32_t)(arg0), (uint32_t)(arg1), (uint32_t)(arg2), (uint32_t)(arg3), (uint32_t)(arg4), (uint32_t)_mock_##fn.arg0History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg1History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg2History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg3History[_mock_##fn.callCount-1], (uint32_t)_mock_##fn.arg4History[_mock_##fn.callCount-1]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }

#define Assert_AllCallsEquals1(fn, arg0) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg0History[_i] != (arg0)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsEquals1 failed: %s, Call: %d, Expected: %d, Actual: %d\n", #fn, _i, (uint32_t)(arg0), (uint32_t)_mock_##fn.arg0History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_AllCallsEquals2(fn, arg1) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg1History[_i] != (arg1)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsEquals2 failed: %s, Call: %d, Expected: %d, Actual: %d, %d\n", #fn, _i, (uint32_t)(arg1), (uint32_t)_mock_##fn.arg0History[_i], (uint32_t)_mock_##fn.arg1History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_AllCallsEquals3(fn, arg2) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg2History[_i] != (arg2)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsEquals3 failed: %s, Call: %d, Expected: %d Actual: %d, %d, %d\n", #fn, _i, (uint32_t)(arg2), (uint32_t)_mock_##fn.arg0History[_i], (uint32_t)_mock_##fn.arg1History[_i], (uint32_t)_mock_##fn.arg2History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_AllCallsEquals4(fn, arg3) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg3History[_i] != (arg3)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsEquals4 failed: %s, Call: %d, Expected: %d Actual: %d, %d, %d, %d\n", #fn, _i, (uint32_t)(arg3), (uint32_t)_mock_##fn.arg0History[_i], (uint32_t)_mock_##fn.arg1History[_i], (uint32_t)_mock_##fn.arg2History[_i], _mock_##fn.arg3History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_AllCallsEquals5(fn, arg4) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg4History[_i] != (arg4)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsEquals5 failed: %s, Call: %d, Expected: %d Actual: %d, %d, %d, %d, %d\n", #fn, _i, (uint32_t)(arg4), (uint32_t)_mock_##fn.arg0History[_i], (uint32_t)_mock_##fn.arg1History[_i], (uint32_t)_mock_##fn.arg2History[_i], (uint32_t)_mock_##fn.arg3History[_i], (uint32_t)_mock_##fn.arg4History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_AllCallsLessThan1(fn, arg0) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg0History[_i] >= (arg0)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsLessThan1 failed: %s, Call: %d, Expected: %d, Actual: %d\n", #fn, _i, (uint32_t)(arg0), (uint32_t)_mock_##fn.arg0History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_AllCallsLessThan2(fn, arg1) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg1History[_i] >= (arg1)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsLessThan2 failed: %s, Call: %d, Expected: %d, Actual: %d\n", #fn, _i, (uint32_t)(arg1), (uint32_t)_mock_##fn.arg0History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_AllCallsLessThan3(fn, arg2) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg2History[_i] >= (arg2)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsLessThan3 failed: %s, Call: %d, Expected: %d, Actual: %d\n", #fn, _i, (uint32_t)(arg2), (uint32_t)_mock_##fn.arg0History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_AllCallsLessThan4(fn, arg3) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg3History[_i] >= (arg3)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsLessThan4 failed: %s, Call: %d, Expected: %d, Actual: %d\n", #fn, _i, (uint32_t)(arg3), (uint32_t)_mock_##fn.arg0History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_AllCallsLessThan5(fn, arg4) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg4History[_i] >= (arg4)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsLessThan5 failed: %s, Call: %d, Expected: %d, Actual: %d\n", #fn, _i, (uint32_t)(arg4), (uint32_t)_mock_##fn.arg0History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_AllCallsGreaterThan1(fn, arg0) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg0History[_i] <= (arg0)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsGreaterThan1 failed: %s, Call: %d, Expected: %d, Actual: %d\n", #fn, _i, (uint32_t)(arg0), (uint32_t)_mock_##fn.arg0History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_AllCallsGreaterThan2(fn, arg1) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg1History[_i] <= (arg1)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsGreaterThan2 failed: %s, Call: %d, Expected: %d, Actual: %d\n", #fn, _i, (uint32_t)(arg1), (uint32_t)_mock_##fn.arg0History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_AllCallsGreaterThan3(fn, arg2) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg2History[_i] <= (arg2)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsGreaterThan3 failed: %s, Call: %d, Expected: %d, Actual: %d\n", #fn, _i, (uint32_t)(arg2), (uint32_t)_mock_##fn.arg0History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_AllCallsGreaterThan4(fn, arg3) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg3History[_i] <= (arg3)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsGreaterThan4 failed: %s, Call: %d, Expected: %d, Actual: %d\n", #fn, _i, (uint32_t)(arg3), (uint32_t)_mock_##fn.arg0History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#define Assert_AllCallsGreaterThan5(fn, arg4) ;{ \
    bool _match= true; \
    uint32_t _i; \
    for (_i= 0; _i < _mock_##fn.callCount; _i++) { \
        if(_mock_##fn.arg4History[_i] <= (arg4)) { \
            _match= false; \
            break; \
        } \
    } \
    if(!_match) { \
        _Mock_PrintFileLine(); \
        printf("Assert_AllCallsGreaterThan5 failed: %s, Call: %d, Expected: %d, Actual: %d\n", #fn, _i, (uint32_t)(arg4), (uint32_t)_mock_##fn.arg0History[_i]); \
        _Mock_Exit(); \
    } else { \
        _assertions++; \
    }} \

#endif
