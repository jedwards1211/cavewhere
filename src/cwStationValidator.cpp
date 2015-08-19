/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

//Our include
#include "cwStationValidator.h"

//Qt includes
#include <QRegExpValidator>

cwStationValidator::cwStationValidator(QObject *parent) :
    cwValidator(parent)
{
    setErrorText("Oops, you haven't entered a valid station name. <br> Station names need to be combination of <b> letters and numbers </b>");
}

QValidator::State cwStationValidator::validate( QString & input, int & pos ) const {
    if(input.isEmpty()) {
        return QValidator::Acceptable;
    }

    QRegExpValidator validator;

    validator.setRegExp(validCharactersRegex());
    return validator.validate(input, pos);
}

int cwStationValidator::validate( QString input ) const {
    int pos = 0;
    return (int)validate(input, pos);
}

/**
 * @brief cwStationValidator::validCharactersRegex
 * @return Retruns the regex that makes up a valid station name
 */
QRegExp cwStationValidator::validCharactersRegex()
{
   return QRegExp("(?:[-_A-Z0-9])+");
}

/**
 * @brief cwStationValidator::invalidCharactersRegex
 * @return Return the regex that will match invalid station character names
 *
 * This is the inverse of validCharactersRegex()
 */
QRegExp cwStationValidator::invalidCharactersRegex()
{
    return QRegExp("(?![^-_A-Z0-9])*");
}
