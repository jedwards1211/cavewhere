/**************************************************************************
**
**    Copyright (C) 2013 by Philip Schuchardt
**    www.cavewhere.com
**
**************************************************************************/

#include "cwTeamMember.h"

cwTeamMember::cwTeamMember() {
}

cwTeamMember::cwTeamMember(QString name, QStringList roles) {
    Name = name;
    Jobs = roles;
}

/**
  Sets the name of a team member of a survey trip
  */
void cwTeamMember::setName(QString name) {
    Name = name;
}

/**
  Sets the roles of a team member
  */
void cwTeamMember::setJobs(QStringList roles)  {
    Jobs = roles;
}

void cwTeamMember::merge(cwTeamMember other) {
    if (other.name().compare(Name, Qt::CaseInsensitive) == 0) {
        foreach (QString job, other.jobs()) {
            if (!Jobs.contains(job, Qt::CaseInsensitive)) {
                Jobs << job;
            }
        }
    }
}
