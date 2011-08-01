#include "cwUndoer.h"

cwUndoer::cwUndoer(QUndoStack* stack) :
    UndoStack(NULL)
{
    setUndoStack(stack);
}

/**
  \brief Pushes a undo command onto the undo stack

  If the undo stack hasn't been set, this does nothing

  */
void cwUndoer::pushUndo(QUndoCommand* command) {
    if(UndoStack == NULL) {
        command->redo();
        delete command;
        return;
    }
    UndoStack->push(command);
}

/**
  \brief See QUndoStack::beginMacro() Qt docs for details

  If the undo stack hasn't been set, this does nothing
  */
void cwUndoer::beginUndoMacro(const QString& text) {
    if(UndoStack == NULL) { return; }
    UndoStack->beginMacro(text);
}

/**
  \brief See QUndoStack::endMacro() Qt docs for details

  If the undo stack hasn't been set, this does nothing
  */
void cwUndoer::endUndoMacro() {
    if(UndoStack == NULL) { return; }
    UndoStack->endMacro();
}

/**
  \brief Sets all the child undo stacks as well

  This will set the undo stack recursively through all the children
  */
void cwUndoer::setUndoStack(QUndoStack* undoStack)  {
    if(UndoStack != undoStack) {
        UndoStack = undoStack;
        setUndoStackForChildren();
    }
}
