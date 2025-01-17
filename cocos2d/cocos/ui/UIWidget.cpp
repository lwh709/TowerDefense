/****************************************************************************
Copyright (c) 2013-2014 Chukong Technologies Inc.

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

#include "ui/UIWidget.h"
#include "ui/UILayout.h"
#include "ui/UIHelper.h"
#include "ui/UIButton.h"

NS_CC_BEGIN

	template <class spriteType>
class ShaderSpriteCreator
{
public:
	static spriteType* createSprite(const std::string& filename)
	{
		spriteType* ret = spriteType::create();
		ret->setTexture(filename);
		ret->initShader();
		ret->setBackgroundNotification();
		return ret;
	};
	static spriteType* createWithSpriteFrameNameT(const std::string& framename)
	{
		spriteType* ret = spriteType::createWithSpriteFrameName(framename);
		ret->initShader();
		ret->setBackgroundNotification();
		return ret;
	}
};

class ShaderSprite : public Sprite
{
public:
	ShaderSprite();
	~ShaderSprite();

	virtual void initShader();
	void setBackgroundNotification();

	virtual void draw(Renderer *renderer, const kmMat4 &transform, bool transformUpdated) override;
	void listenBackToForeground(Ref *obj);

protected:
	virtual void buildCustomUniforms() = 0;
	virtual void setCustomUniforms() = 0;
protected:
	std::string _fragSourceFile;

protected:
	CustomCommand _renderCommand;
	void onDraw(const kmMat4 &transform, bool transformUpdated);

};

ShaderSprite::ShaderSprite()
{
}

ShaderSprite::~ShaderSprite()
{
}

void ShaderSprite::setBackgroundNotification()
{
#if CC_ENABLE_CACHE_TEXTURE_DATA
	auto listener = EventListenerCustom::create(EVENT_COME_TO_FOREGROUND, [this](EventCustom* event){
		this->setShaderProgram(nullptr);
		this->initShader();
	});
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
#endif
}

void ShaderSprite::initShader()
{
	GLchar * fragSource = (GLchar*) String::createWithContentsOfFile(
		FileUtils::getInstance()->fullPathForFilename(_fragSourceFile).c_str())->getCString();
	auto program = new GLProgram();
	program->initWithByteArrays(ccPositionTextureColor_vert, fragSource);
	setShaderProgram(program);
	program->release();

	CHECK_GL_ERROR_DEBUG();

	program->bindAttribLocation(GLProgram::ATTRIBUTE_NAME_POSITION, GLProgram::VERTEX_ATTRIB_POSITION);
	program->bindAttribLocation(GLProgram::ATTRIBUTE_NAME_COLOR, GLProgram::VERTEX_ATTRIB_COLOR);
	program->bindAttribLocation(GLProgram::ATTRIBUTE_NAME_TEX_COORD, GLProgram::VERTEX_ATTRIB_TEX_COORDS);

	CHECK_GL_ERROR_DEBUG();

	program->link();

	CHECK_GL_ERROR_DEBUG();

	program->updateUniforms();

	CHECK_GL_ERROR_DEBUG();

	buildCustomUniforms();

	CHECK_GL_ERROR_DEBUG();
}

void ShaderSprite::draw(Renderer *renderer, const kmMat4 &transform, bool transformUpdated)
{
	_renderCommand.init(_globalZOrder);
	_renderCommand.func = CC_CALLBACK_0(ShaderSprite::onDraw, this, transform, transformUpdated);
	renderer->addCommand(&_renderCommand);

}

void ShaderSprite::onDraw(const kmMat4 &transform, bool transformUpdated)
{
	auto shader = getShaderProgram();
	shader->use();
	shader->setUniformsForBuiltins(transform);

	setCustomUniforms();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POS_COLOR_TEX );
	GL::blendFunc(_blendFunc.src, _blendFunc.dst);
	GL::bindTexture2D( getTexture()->getName());

	//
	// Attributes
	//
#define kQuadSize sizeof(_quad.bl)
	size_t offset = (size_t)&_quad;

	// vertex
	int diff = offsetof( V3F_C4B_T2F, vertices);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, kQuadSize, (void*) (offset + diff));

	// texCoods
	diff = offsetof( V3F_C4B_T2F, texCoords);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORDS, 2, GL_FLOAT, GL_FALSE, kQuadSize, (void*)(offset + diff));

	// color
	diff = offsetof( V3F_C4B_T2F, colors);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (void*)(offset + diff));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	CC_INCREMENT_GL_DRAWN_BATCHES_AND_VERTICES(1, 4);
}


class GreySprite : public ShaderSprite, public ShaderSpriteCreator<GreySprite>
{
public:
	CREATE_FUNC(GreySprite);
	static GreySprite* createWithSpriteFrameName(const std::string &frameName);
	static GreySprite* createWithSpriteFrame(SpriteFrame *spriteFrame);

	GreySprite();
protected:
	virtual void buildCustomUniforms();
	virtual void setCustomUniforms();
};

GreySprite::GreySprite()
{
	_fragSourceFile = "Shaders/example_greyScale.fsh";
}

GreySprite* GreySprite::createWithSpriteFrameName(const std::string &frameName)
{
	SpriteFrame *frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(frameName);

#if COCOS2D_DEBUG > 0
	char msg[256] = {0};
	sprintf(msg, "Invalid spriteFrameName: %s", frameName.c_str());
	CCASSERT(frame != nullptr, msg);
#endif

	return createWithSpriteFrame(frame);
}

GreySprite* GreySprite::createWithSpriteFrame(SpriteFrame *spriteFrame)
{
	GreySprite *sprite = new GreySprite();
	if (spriteFrame && sprite && sprite->initWithSpriteFrame(spriteFrame))
	{
		sprite->autorelease();
		return sprite;
	}
	CC_SAFE_DELETE(sprite);
	return nullptr;
}


void GreySprite::buildCustomUniforms()
{

}

void GreySprite::setCustomUniforms()
{

}




namespace ui {

Widget::Widget():
_enabled(true),
_bright(true),
_touchEnabled(false),
_touchPassedEnabled(false),
_focus(false),
_brightStyle(BRIGHT_NONE),
_touchStartPos(Point::ZERO),
_touchMovePos(Point::ZERO),
_touchEndPos(Point::ZERO),
_touchEventListener(nullptr),
_touchEventSelector(nullptr),
_name("default"),
_widgetType(WidgetTypeWidget),
_actionTag(0),
_size(Size::ZERO),
_customSize(Size::ZERO),
_ignoreSize(false),
_affectByClipping(false),
_sizeType(SIZE_ABSOLUTE),
_sizePercent(Point::ZERO),
_positionType(POSITION_ABSOLUTE),
_positionPercent(Point::ZERO),
_reorderWidgetChildDirty(true),
_hitted(false),
_touchListener(nullptr),
_color(Color3B::WHITE),
_opacity(255),
_flippedX(false),
_flippedY(false),
_touchPriority(0),
_isgrey(false)
{

}

Widget::~Widget()
{
    _touchEventListener = nullptr;
    _touchEventSelector = nullptr;
    setTouchEnabled(false);
}

Widget* Widget::create()
{
    Widget* widget = new Widget();
    if (widget && widget->init())
    {
        widget->autorelease();
        return widget;
    }
    CC_SAFE_DELETE(widget);
    return nullptr;
}

bool Widget::init()
{
    if (ProtectedNode::init())
    {
        initRenderer();
        setBright(true);
        ignoreContentAdaptWithSize(true);
        setAnchorPoint(Point(0.5f, 0.5f));
		setFrameName("");
        return true;
    }
    return false;
}

void Widget::onEnter()
{
    updateSizeAndPosition();
    ProtectedNode::onEnter();
}

void Widget::onExit()
{
    unscheduleUpdate();
    ProtectedNode::onExit();
}

void Widget::visit(Renderer *renderer, const kmMat4 &parentTransform, bool parentTransformUpdated)
{
    if (_enabled)
    {
        adaptRenderers();
        ProtectedNode::visit(renderer, parentTransform, parentTransformUpdated);
    }
}

Widget* Widget::getWidgetParent()
{
    return dynamic_cast<Widget*>(getParent());
}

void Widget::setEnabled(bool enabled)
{
    _enabled = enabled;
    for (auto& child : _children)
    {
        if (child)
        {
            Widget* widgetChild = dynamic_cast<Widget*>(child);
            if (widgetChild)
            {
                widgetChild->setEnabled(enabled);
            }
        }
    }
    
    for (auto& child : _protectedChildren)
    {
        if (child)
        {
            Widget* widgetChild = dynamic_cast<Widget*>(child);
            if (widgetChild)
            {
                widgetChild->setEnabled(enabled);
            }
        }
    }
}

Widget* Widget::getChildByName(const char *name)
{
    for (auto& child : _children)
    {
        if (child)
        {
            Widget* widgetChild = dynamic_cast<Widget*>(child);
            if (widgetChild)
            {
                if (strcmp(widgetChild->getName(), name) == 0)
                {
                    return widgetChild;
                }
            }
        }
    }
    return nullptr;
}
    
void Widget::initRenderer()
{
}

void Widget::setSize(const Size &size)
{
    _customSize = size;
    if (_ignoreSize)
    {
        _size = getVirtualRendererSize();
    }
    else
    {
        _size = size;
    }
    if (_running)
    {
        Widget* widgetParent = getWidgetParent();
        Size pSize;
        if (widgetParent)
        {
            pSize = widgetParent->getSize();
        }
        else
        {
            pSize = _parent->getContentSize();
        }
        float spx = 0.0f;
        float spy = 0.0f;
        if (pSize.width > 0.0f)
        {
            spx = _customSize.width / pSize.width;
        }
        if (pSize.height > 0.0f)
        {
            spy = _customSize.height / pSize.height;
        }
        _sizePercent = Point(spx, spy);
    }
    onSizeChanged();
}

void Widget::setSizePercent(const Point &percent)
{
    _sizePercent = percent;
    Size cSize = _customSize;
    if (_running)
    {
        Widget* widgetParent = getWidgetParent();
        if (widgetParent)
        {
            cSize = Size(widgetParent->getSize().width * percent.x , widgetParent->getSize().height * percent.y);
        }
        else
        {
            cSize = Size(_parent->getContentSize().width * percent.x , _parent->getContentSize().height * percent.y);
        }
    }
    if (_ignoreSize)
    {
        _size = getVirtualRendererSize();
    }
    else
    {
        _size = cSize;
    }
    _customSize = cSize;
    onSizeChanged();
}

void Widget::updateSizeAndPosition()
{
    Widget* widgetParent = getWidgetParent();
    Size pSize;
    if (widgetParent)
    {
        pSize = widgetParent->getLayoutSize();
    }
    else
    {
        pSize = _parent->getContentSize();
    }
    updateSizeAndPosition(pSize);
}
    
void Widget::updateSizeAndPosition(const cocos2d::Size &parentSize)
{
    switch (_sizeType)
    {
        case SIZE_ABSOLUTE:
        {
            if (_ignoreSize)
            {
                _size = getVirtualRendererSize();
            }
            else
            {
                _size = _customSize;
            }
            float spx = 0.0f;
            float spy = 0.0f;
            if (parentSize.width > 0.0f)
            {
                spx = _customSize.width / parentSize.width;
            }
            if (parentSize.height > 0.0f)
            {
                spy = _customSize.height / parentSize.height;
            }
            _sizePercent = Point(spx, spy);
            break;
        }
        case SIZE_PERCENT:
        {
            Size cSize = Size(parentSize.width * _sizePercent.x , parentSize.height * _sizePercent.y);
            if (_ignoreSize)
            {
                _size = getVirtualRendererSize();
            }
            else
            {
                _size = cSize;
            }
            _customSize = cSize;
            break;
        }
        default:
            break;
    }
    onSizeChanged();
    Point absPos = getPosition();
    switch (_positionType)
    {
        case POSITION_ABSOLUTE:
        {
            if (parentSize.width <= 0.0f || parentSize.height <= 0.0f)
            {
                _positionPercent = Point::ZERO;
            }
            else
            {
                _positionPercent = Point(absPos.x / parentSize.width, absPos.y / parentSize.height);
            }
            break;
        }
        case POSITION_PERCENT:
        {
            absPos = Point(parentSize.width * _positionPercent.x, parentSize.height * _positionPercent.y);
            break;
        }
        default:
            break;
    }
    setPosition(absPos);
}

void Widget::setSizeType(SizeType type)
{
    _sizeType = type;
}

SizeType Widget::getSizeType() const
{
    return _sizeType;
}

void Widget::ignoreContentAdaptWithSize(bool ignore)
{
    if (_ignoreSize == ignore)
    {
        return;
    }
    _ignoreSize = ignore;
    if (_ignoreSize)
    {
        Size s = getVirtualRendererSize();
        _size = s;
    }
    else
    {
        _size = _customSize;
    }
    onSizeChanged();
}

bool Widget::isIgnoreContentAdaptWithSize() const
{
    return _ignoreSize;
}

const Size& Widget::getSize() const
{
    return _size;
}
    
const Size& Widget::getCustomSize() const
{
    return _customSize;
}

const Point& Widget::getSizePercent() const
{
    return _sizePercent;
}

Point Widget::getWorldPosition()
{
    return convertToWorldSpace(Point(_anchorPoint.x * _contentSize.width, _anchorPoint.y * _contentSize.height));
}

Node* Widget::getVirtualRenderer()
{
    return this;
}

void Widget::onSizeChanged()
{
    setContentSize(_size);
    for (auto& child : getChildren())
    {
        Widget* widgetChild = dynamic_cast<Widget*>(child);
        if (widgetChild)
        {
            widgetChild->updateSizeAndPosition();
        }
    }
}

const Size& Widget::getVirtualRendererSize() const
{
    return _contentSize;
}
    
void Widget::updateContentSizeWithTextureSize(const cocos2d::Size &size)
{
    if (_ignoreSize)
    {
        _size = size;
    }
    else
    {
        _size = _customSize;
    }
    onSizeChanged();
}

void Widget::setTouchEnabled(bool enable)
{
    if (enable == _touchEnabled)
    {
        return;
    }
    _touchEnabled = enable;
    if (_touchEnabled)
    {
        _touchListener = EventListenerTouchOneByOne::create();
        CC_SAFE_RETAIN(_touchListener);
        _touchListener->setSwallowTouches(true);
        _touchListener->onTouchBegan = CC_CALLBACK_2(Widget::onTouchBegan, this);
        _touchListener->onTouchMoved = CC_CALLBACK_2(Widget::onTouchMoved, this);
        _touchListener->onTouchEnded = CC_CALLBACK_2(Widget::onTouchEnded, this);
        _touchListener->onTouchCancelled = CC_CALLBACK_2(Widget::onTouchCancelled, this);
        _eventDispatcher->addEventListenerWithSceneGraphPriority(_touchListener, this);

		if (_touchPriority!=0){
			setTouchPriority(_touchPriority);
		}
    }
    else
    {
        _eventDispatcher->removeEventListener(_touchListener);
        CC_SAFE_RELEASE_NULL(_touchListener);
    }
}

bool Widget::isTouchEnabled() const
{
    return _touchEnabled;
}

bool Widget::isFocused() const
{
    return _focus;
}

void Widget::setFocused(bool fucos)
{
    if (fucos == _focus)
    {
        return;
    }
    _focus = fucos;
    if (_bright)
    {
        if (_focus)
        {
            setBrightStyle(BRIGHT_HIGHLIGHT);
        }
        else
        {
            setBrightStyle(BRIGHT_NORMAL);
        }
    }
    else
    {
        onPressStateChangedToDisabled();
    }
}

void Widget::setBright(bool bright)
{
    _bright = bright;
    if (_bright)
    {
        _brightStyle = BRIGHT_NONE;
        setBrightStyle(BRIGHT_NORMAL);
    }
    else
    {
        onPressStateChangedToDisabled();
    }
}

void Widget::setBrightStyle(BrightStyle style)
{
    if (_brightStyle == style)
    {
        return;
    }
    _brightStyle = style;
    switch (_brightStyle)
    {
        case BRIGHT_NORMAL:
            onPressStateChangedToNormal();
            break;
        case BRIGHT_HIGHLIGHT:
            onPressStateChangedToPressed();
            break;
        default:
            break;
    }
}

void Widget::onPressStateChangedToNormal()
{

}

void Widget::onPressStateChangedToPressed()
{

}

void Widget::onPressStateChangedToDisabled()
{

}

void Widget::didNotSelectSelf()
{

}

bool Widget::onTouchBegan(Touch *touch, Event *unusedEvent)
{
    _hitted = false;
    if (isEnabled() && isTouchEnabled())
    {
        _touchStartPos = touch->getLocation();
        if(hitTest(_touchStartPos) && clippingParentAreaContainPoint(_touchStartPos))
        {
            _hitted = true;
        }
    }
    if (!_hitted)
    {
        return false;
    }
    setFocused(true);
    Widget* widgetParent = getWidgetParent();
    if (widgetParent)
    {
        widgetParent->checkChildInfo(0,this,_touchStartPos);
    }
    pushDownEvent();
    return !_touchPassedEnabled;
}

void Widget::onTouchMoved(Touch *touch, Event *unusedEvent)
{
    _touchMovePos = touch->getLocation();
    setFocused(hitTest(_touchMovePos));
    Widget* widgetParent = getWidgetParent();
    if (widgetParent)
    {
        widgetParent->checkChildInfo(1,this,_touchMovePos);
    }
    moveEvent();
}

void Widget::onTouchEnded(Touch *touch, Event *unusedEvent)
{
    _touchEndPos = touch->getLocation();
    bool focus = _focus;
    setFocused(false);
    Widget* widgetParent = getWidgetParent();
    if (widgetParent)
    {
        widgetParent->checkChildInfo(2,this,_touchEndPos);
    }
    if (focus)
    {
        releaseUpEvent();
    }
    else
    {
        cancelUpEvent();
    }
}

void Widget::onTouchCancelled(Touch *touch, Event *unusedEvent)
{
    setFocused(false);
    cancelUpEvent();
}

void Widget::pushDownEvent()
{
    if (_touchEventListener && _touchEventSelector)
    {
        (_touchEventListener->*_touchEventSelector)(this,TOUCH_EVENT_BEGAN);
    }
}

void Widget::moveEvent()
{
    if (_touchEventListener && _touchEventSelector)
    {
        (_touchEventListener->*_touchEventSelector)(this,TOUCH_EVENT_MOVED);
    }
}

void Widget::releaseUpEvent()
{
    if (_touchEventListener && _touchEventSelector)
    {
        (_touchEventListener->*_touchEventSelector)(this,TOUCH_EVENT_ENDED);
    }
}

void Widget::cancelUpEvent()
{
    if (_touchEventListener && _touchEventSelector)
    {
        (_touchEventListener->*_touchEventSelector)(this,TOUCH_EVENT_CANCELED);
    }
}

void Widget::addTouchEventListener(Ref *target, SEL_TouchEvent selector)
{
    _touchEventListener = target;
    _touchEventSelector = selector;
}

void Widget::removeTouchEventListener()
{
	_touchEventListener = nullptr;
	_touchEventSelector = nullptr;
}

bool Widget::hitTest(const Point &pt)
{
    Point nsp = convertToNodeSpace(pt);
    Rect bb;
    bb.size = _contentSize;
    if (bb.containsPoint(nsp))
    {
        return true;
    }
    return false;
}

bool Widget::clippingParentAreaContainPoint(const Point &pt)
{
    _affectByClipping = false;
    Widget* parent = getWidgetParent();
    Widget* clippingParent = nullptr;
    while (parent)
    {
        Layout* layoutParent = dynamic_cast<Layout*>(parent);
        if (layoutParent)
        {
            if (layoutParent->isClippingEnabled())
            {
                _affectByClipping = true;
                clippingParent = layoutParent;
                break;
            }
        }
        parent = parent->getWidgetParent();
    }

    if (!_affectByClipping)
    {
        return true;
    }


    if (clippingParent)
    {
        bool bRet = false;
        if (clippingParent->hitTest(pt))
        {
            bRet = true;
        }
        if (bRet)
        {
            return clippingParent->clippingParentAreaContainPoint(pt);
        }
        return false;
    }
    return true;
}

void Widget::checkChildInfo(int handleState, Widget *sender, const Point &touchPoint)
{
    Widget* widgetParent = getWidgetParent();
    if (widgetParent)
    {
        widgetParent->checkChildInfo(handleState,sender,touchPoint);
    }
}

void Widget::setPosition(const Point &pos)
{
    if (_running)
    {
        Widget* widgetParent = getWidgetParent();
        if (widgetParent)
        {
            Size pSize = widgetParent->getSize();
            if (pSize.width <= 0.0f || pSize.height <= 0.0f)
            {
                _positionPercent = Point::ZERO;
            }
            else
            {
                _positionPercent = Point(pos.x / pSize.width, pos.y / pSize.height);
            }
        }
    }
    ProtectedNode::setPosition(pos);
}

void Widget::setPositionPercent(const Point &percent)
{
    _positionPercent = percent;
    if (_running)
    {
        Widget* widgetParent = getWidgetParent();
        if (widgetParent)
        {
            Size parentSize = widgetParent->getSize();
            Point absPos = Point(parentSize.width * _positionPercent.x, parentSize.height * _positionPercent.y);
            setPosition(absPos);
        }
    }
}

const Point& Widget::getPositionPercent()
{
    return _positionPercent;
}

void Widget::setPositionType(PositionType type)
{
    _positionType = type;
}

PositionType Widget::getPositionType() const
{
    return _positionType;
}

bool Widget::isBright() const
{
    return _bright;
}

bool Widget::isEnabled() const
{
    return _enabled;
}

float Widget::getLeftInParent()
{
    return getPosition().x - getAnchorPoint().x * _size.width;;
}

float Widget::getBottomInParent()
{
    return getPosition().y - getAnchorPoint().y * _size.height;;
}

float Widget::getRightInParent()
{
    return getLeftInParent() + _size.width;
}

float Widget::getTopInParent()
{
    return getBottomInParent() + _size.height;
}

const Point& Widget::getTouchStartPos()
{
    return _touchStartPos;
}

const Point& Widget::getTouchMovePos()
{
    return _touchMovePos;
}

const Point& Widget::getTouchEndPos()
{
    return _touchEndPos;
}

void Widget::setName(const char* name)
{
    _name = name;
}

const char* Widget::getName() const
{
    return _name.c_str();
}

WidgetType Widget::getWidgetType() const
{
    return _widgetType;
}

void Widget::setLayoutParameter(LayoutParameter *parameter)
{
    if (!parameter)
    {
        return;
    }
    _layoutParameterDictionary.insert(parameter->getLayoutType(), parameter);
}

LayoutParameter* Widget::getLayoutParameter(LayoutParameterType type)
{
    return dynamic_cast<LayoutParameter*>(_layoutParameterDictionary.at(type));
}

std::string Widget::getDescription() const
{
    return "Widget";
}

Widget* Widget::clone()
{
    Widget* clonedWidget = createCloneInstance();
    clonedWidget->copyProperties(this);
    clonedWidget->copyClonedWidgetChildren(this);
    return clonedWidget;
}

Widget* Widget::createCloneInstance()
{
    return Widget::create();
}

void Widget::copyClonedWidgetChildren(Widget* model)
{
    auto& modelChildren = model->getChildren();

    for (auto& subWidget : modelChildren)
    {
        Widget* child = dynamic_cast<Widget*>(subWidget);
        if (child)
        {
            addChild(child->clone());
        }
    }
}

void Widget::copySpecialProperties(Widget* model)
{

}

void Widget::copyProperties(Widget *widget)
{
    setEnabled(widget->isEnabled());
    setVisible(widget->isVisible());
    setBright(widget->isBright());
    setTouchEnabled(widget->isTouchEnabled());
    _touchPassedEnabled = false;
    setLocalZOrder(widget->getLocalZOrder());
    setTag(widget->getTag());
    setName(widget->getName());
    setActionTag(widget->getActionTag());
    _ignoreSize = widget->_ignoreSize;
    _size = widget->_size;
    _customSize = widget->_customSize;
    copySpecialProperties(widget);
    _sizeType = widget->getSizeType();
    _sizePercent = widget->_sizePercent;
    _positionType = widget->_positionType;
    _positionPercent = widget->_positionPercent;
    setPosition(widget->getPosition());
    setAnchorPoint(widget->getAnchorPoint());
    setScaleX(widget->getScaleX());
    setScaleY(widget->getScaleY());
    setRotation(widget->getRotation());
    setRotationSkewX(widget->getRotationSkewX());
    setRotationSkewY(widget->getRotationSkewY());
    setFlippedX(widget->isFlippedX());
    setFlippedY(widget->isFlippedY());
    setColor(widget->getColor());
    setOpacity(widget->getOpacity());
    Map<int, LayoutParameter*>& layoutParameterDic = widget->_layoutParameterDictionary;
    for (auto iter = layoutParameterDic.begin(); iter != layoutParameterDic.end(); ++iter)
    {
        setLayoutParameter(iter->second->clone());
    }
    onSizeChanged();
}
    
void Widget::setColor(const Color3B& color)
{
    _color = color;
    updateTextureColor();
}

void Widget::setOpacity(GLubyte opacity)
{
    _opacity = opacity;
    updateTextureOpacity();
}
    
void Widget::setFlippedX(bool flippedX)
{
    _flippedX = flippedX;
    updateFlippedX();
}

void Widget::setFlippedY(bool flippedY)
{
    _flippedY = flippedY;
    updateFlippedY();
}

void Widget::updateColorToRenderer(Node* renderer)
{
    renderer->setColor(_color);
}

void Widget::updateOpacityToRenderer(Node* renderer)
{
    renderer->setOpacity(_opacity);
}

void Widget::updateRGBAToRenderer(Node* renderer)
{
    renderer->setColor(_color);
    renderer->setOpacity(_opacity);
}

/*temp action*/
void Widget::setActionTag(int tag)
{
	_actionTag = tag;
}

int Widget::getActionTag()
{
	return _actionTag;
}

void Widget::setTouchPriority(int priority)
{
	_touchPriority = priority;

	for (auto& child : _children)
	{
		if (child)
		{
			Widget* widgetChild = dynamic_cast<Widget*>(child);
			if (widgetChild)
			{
				widgetChild->setTouchPriority(this->getTouchPriority()-1);
			}
		}
	}

	if (!_touchEnabled)
		return;
	
	_eventDispatcher->removeEventListener(_touchListener);
	CC_SAFE_RELEASE_NULL(_touchListener);

	_touchListener = EventListenerTouchOneByOne::create();
	CC_SAFE_RETAIN(_touchListener);
	_touchListener->setSwallowTouches(true);
	_touchListener->onTouchBegan = CC_CALLBACK_2(Widget::onTouchBegan, this);
	_touchListener->onTouchMoved = CC_CALLBACK_2(Widget::onTouchMoved, this);
	_touchListener->onTouchEnded = CC_CALLBACK_2(Widget::onTouchEnded, this);
	_touchListener->onTouchCancelled = CC_CALLBACK_2(Widget::onTouchCancelled, this);
	_eventDispatcher->addEventListenerWithFixedPriority(_touchListener, _touchPriority);

}

int Widget::getTouchPriority()
{
	return _touchPriority;
}

void Widget::setGrey(float dt)
{
	auto ch = getChildByTag(1777);
	if (ch) 
		ch->removeFromParentAndCleanup(true);

	for (auto& child : _children)
	{
		if (child)
		{
			Widget* widgetChild = dynamic_cast<Widget*>(child);
			if (widgetChild!=nullptr)
			{
				widgetChild->setLocalZOrder(widgetChild->getLocalZOrder()+1);
				if (widgetChild && widgetChild->getDescription()!="Button" )
				{
					widgetChild->setGrey(dt);
				}else if (widgetChild->getDescription()=="Button"){
					Button *btn = static_cast<Button*>(widgetChild);
					btn->setGrey(dt);
				}
			}
			
			
		}
	}
	loadGreyTexture(dt);
}

void Widget::loadGreyTexture(float dt)
{
	auto ch = getChildByTag(1777);
	if (ch) 
		ch->removeFromParentAndCleanup(true);

	std::string frameName = getFrameName();
	if (frameName=="")
		return;

	GreySprite *sp;

	switch (getResType())
	{
	case UI_TEX_TYPE_PLIST:
		sp = GreySprite::createWithSpriteFrameNameT(frameName);
		break;
	case UI_TEX_TYPE_LOCAL:
		sp = GreySprite::createSprite(frameName);
		break;
	default:
		break;
	}
	sp->setPosition(Point( (0.5)*this->getSize().width ,(0.5)*this->getSize().height ));
	sp->setTag(1777);
	sp->setScaleX(getSize().width/sp->getContentSize().width+dt);
	sp->setScaleY(getSize().height/sp->getContentSize().height+dt);
	sp->setAnchorPoint(Point::ANCHOR_MIDDLE);

	addChild(sp,0);

	_isgrey=true;
}

void Widget::removeGrey()
{
	auto child = getChildByTag(1777);
	if (!child)  return;

	child->removeFromParentAndCleanup(true);
	for (auto& child : _children)
	{
		if (child)
		{
			Widget* widgetChild = dynamic_cast<Widget*>(child);
			if (widgetChild!=nullptr)
			{
				widgetChild->setLocalZOrder(widgetChild->getLocalZOrder()-1);
				if (widgetChild && widgetChild->getDescription()!="Button" )
				{
					widgetChild->removeGrey();
				}else if (widgetChild->getDescription()=="Button"){
					Button *btn = static_cast<Button*>(widgetChild);
					btn->removeGrey();
				}

			}

			
		}
	}

	_isgrey=false;
}

void Widget::greyFadeOut(float duration)
{
	for (auto& child : _children)
	{
		if (child)
		{
			Widget* widgetChild = static_cast<Widget*>(child);
			if ( widgetChild->getTag() == 1777 && widgetChild->_isgrey){
				//������Ч
				widgetChild->runAction(Sequence::create(FadeOut::create(duration),
											   CallFuncN::create(CC_CALLBACK_1(Widget::greyFadeOutCallFunc,this)),
											   NULL
								               ));
			}
			widgetChild->greyFadeOut(duration);
		}
	}

}

void Widget::greyFadeOutCallFunc(Object *pSender)
{
	Widget *wt = static_cast<Widget*>(pSender);
	wt->removeFromParentAndCleanup(true);

	if (getDescription()=="Button"){
		setTouchEnabled(true);
	}
	_isgrey=false;
}


}

NS_CC_END
