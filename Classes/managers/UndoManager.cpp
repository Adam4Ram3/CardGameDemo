#include "managers/UndoManager.h"

UndoManager::UndoManager() {
    // 构造函数：初始化时栈是空的，无需特殊操作
}

UndoManager::~UndoManager() {
    // 析构函数：栈会自动释放内存
}

void UndoManager::pushCommand(const UndoCommand& cmd) {
    _history.push(cmd);
}

bool UndoManager::canUndo() const {
    return !_history.empty();
}

UndoCommand UndoManager::popCommand() {
    if (_history.empty()) {
        // 如果栈空了，返回一个无效的默认命令
        // (实际上 Controller 调用前应该先检查 canUndo)
        return UndoCommand();
    }
    
    UndoCommand cmd = _history.top();
    _history.pop();
    return cmd;
}