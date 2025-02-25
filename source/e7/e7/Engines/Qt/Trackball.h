/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef TRACKBALL_H
#define TRACKBALL_H

#include <QtGui>

#include <QtGui/qvector3d.h>
#include <QtGui/qquaternion.h>

class Trackball
{
public:
    enum TrackMode
    {
        Plane,
        Sphere,
    };
    Trackball(TrackMode mode = Sphere);
    Trackball(float angularVelocity, const QVector3D& axis, TrackMode mode = Sphere);
    // coordinates in [-1,1]x[-1,1]
    void push(const QPointF& p, const QQuaternion &transformation);
    void move(const QPointF& p, const QQuaternion &transformation);
    void release(const QPointF& p, const QQuaternion &transformation);
    void start(); // starts clock
    void stop(); // stops clock
    QQuaternion rotation() const;

	void setRotation( QQuaternion q ) { m_rotation = q; }

private:
    QQuaternion m_rotation;
    QVector3D m_axis;
    float m_angularVelocity;

    QPointF m_lastPos;
    QTime m_lastTime;
    bool m_paused;
    bool m_pressed;
    TrackMode m_mode;
};

#endif
