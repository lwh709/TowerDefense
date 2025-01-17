/****************************************************************************
Copyright (C) 2010      Lam Pham
Copyright (c) 2010-2012 cocos2d-x.org
CopyRight (c) 2013-2014 Chukong Technologies Inc.
 
http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#ifndef __ACTION_LOADINGBAR_TIMER_H__
#define __ACTION_LOADINGBAR_TIMER_H__

#include "CCActionInterval.h"

NS_CC_BEGIN

/**
 * @addtogroup actions
 * @{
 */

/**
@brief Loading to percentage
@since v0.99.1
*/
class CC_DLL LoadingTo : public ActionInterval
{
public:
    /** Creates and initializes with a duration and a percent */
    static LoadingTo* create(float duration, float percent);

    //
    // Overrides
    //
	virtual LoadingTo* clone() const override;
	virtual LoadingTo* reverse(void) const override;
    virtual void startWithTarget(Node *target) override;
    virtual void update(float time) override;

protected:
    LoadingTo() {}
    virtual ~LoadingTo() {}
    /** Initializes with a duration and a percent */
    bool initWithDuration(float duration, float percent);

    float _to;
    float _from;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(LoadingTo);
};

/**
@brief Loading from a percentage to another percentage
@since v0.99.1
*/
class CC_DLL LoadingFromTo : public ActionInterval
{
public:
    /** Creates and initializes the action with a duration, a "from" percentage and a "to" percentage */
    static LoadingFromTo* create(float duration, float fromPercentage, float toPercentage);

    //
    // Overrides
    //
	virtual LoadingFromTo* clone() const override;
	virtual LoadingFromTo* reverse(void) const override;
    virtual void startWithTarget(Node *target) override;
    virtual void update(float time) override;

protected:
    LoadingFromTo() {}
    virtual ~LoadingFromTo() {}
    /** Initializes the action with a duration, a "from" percentage and a "to" percentage */
    bool initWithDuration(float duration, float fromPercentage, float toPercentage);

    float _to;
    float _from;

private:
    CC_DISALLOW_COPY_AND_ASSIGN(LoadingFromTo);
};

// end of actions group
/// @}

NS_CC_END

#endif // __ACTION_Loading_TIMER_H__
