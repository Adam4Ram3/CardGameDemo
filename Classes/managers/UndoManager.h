#ifndef UNDO_MANAGER_H
#define UNDO_MANAGER_H

#include "models/UndoModel.h"
#include <stack>
#include <memory>

/**
 * @brief 回退管理器
 * 职责：维护一个操作历史栈，提供压栈(Push)和出栈(Pop)功能
 */
class UndoManager {
public:
    UndoManager();
    ~UndoManager();

    // 记录一步操作
    void pushCommand(const UndoCommand& cmd);

    // 检查是否有可回退的操作
    bool canUndo() const;

    // 取出并移除最近的一步操作
    UndoCommand popCommand();

private:
    std::stack<UndoCommand> _history;
};

#endif // UNDO_MANAGER_H