// IState.h
#pragma once
#include "event.h"

// 向前声明, 避免头文件循环依赖
namespace mcompass {
    class Context; 
}

class IState {
public:
    virtual ~IState() {}

    /**
     * @brief 当状态被激活时调用 (进入此状态)
     */
    virtual void onEnter(mcompass::Context& context) = 0;

    /**
     * @brief 当状态被停用时调用 (离开此状态)
     */
    virtual void onExit(mcompass::Context& context) = 0;

    /**
     * @brief 处理在此状态下发生的事件
     */
    virtual void handleEvent(mcompass::Context& context, Event::Body* evt) = 0;

    /**
     * @brief [可选] 返回状态的名称, 方便调试
     */
    virtual const char* getName() = 0;
};