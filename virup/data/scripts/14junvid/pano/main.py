from PythonQt.QtGui import QKeyEvent
from PythonQt.QtCore import QElapsedTimer
from PythonQt.QtCore import Qt
from PythonQt.QtCore import QDateTime
from PythonQt.QtCore import QDate
from PythonQt.QtCore import QTime
from PythonQt.libplanet import Vector3
from math import exp
from math import log
from math import cos
from math import sin


def smoothstep(t, v0 = 0, v1 = 0):
    if t < 0.0:
        return 0.0
    if t > 1.0:
        return 1.0
    return (6*t**5 - 15*t**4 + 10*t**3 + 0.5*(v1-v0)*t**2 + v0*t) / (1.0 + 0.5*(v1+v0))



class SpatialData:
    def __init__(self, cosmoPos, invscale, bodyName = '', systemName = '', planetPos = Vector3()):
        self.cosmoPos = cosmoPos
        self.scale = 1.0 / invscale
        self.bodyName = bodyName
        self.systemName = systemName
        self.planetPos = planetPos

class TemporalData:
    def __init__(self, timeCoeff = 1.0, simulationTime = QDateTime()):
        self.timeCoeff = timeCoeff
        self.simulationTime = simulationTime

class UI:
    def __init__(self, sdsslum=1.0, gaialum=1.0, illustrislum=1.0, agoralum=1.0):
        self.sdsslum = sdsslum 
        self.gaialum = gaialum 
        self.illustrislum = illustrislum 
        self.agoralum = agoralum

class Scene:
    def __init__(self, spatialData, temporalData = TemporalData(), ui = UI()):
        self.spatialData = spatialData
        self.temporalData = temporalData
        self.ui = ui


# interpolate functions between 0 and 1 with continuous parameter t from 0 to 1

def interpolateBool(b0, b1, t):
    if t < 0.5:
        return b0
    else:
        return b1

def interpolateLinear(x0, x1, t):
    return x0 * (1 - t) + x1 * t

def interpolateLog(x0, x1, t):
    return exp(log(x0) * (1 - t) + log(x1) * t)

def interpolateDateTime(dt0, dt1, t):
    global currentscene
    global longanimation

    if not dt1.isValid():
        return QDateTime()
    ms0 = currentscene.temporalData.simulationTime.toMSecsSinceEpoch()
    ms1 = dt1.toMSecsSinceEpoch()
    ms = ms0 * (1-t) + ms1 * t
    if longanimation:
        if t <= 0.25:
            t=0.0
        elif t <= 0.75:
            t=2.0*t - 0.5
        else:
            t=1.0
    ms = ms0 * (1-t) + ms1 * t
    return QDateTime.fromMSecsSinceEpoch(ms, Qt.UTC)

def interpolateSpatialData(s0, s1, t, simTime0, simTime1):
    global longanimation

    longanimation = False
    planetpos = Vector3()

    scale=1.0 / interpolateLog(s0.scale, s1.scale, t)

    dist=(s0.cosmoPos - s1.cosmoPos).length() * 3.086e+19
    if s0.systemName != s1.systemName and dist > 1e9:
        if t <= 0.25:
            inter0=SpatialData(s0.cosmoPos, dist)
            result=interpolateSpatialData(s0, inter0, t*4, simTime0, simTime0)
        elif t <= 0.75:
            inter0=SpatialData(s0.cosmoPos, dist)
            inter1=SpatialData(s1.cosmoPos, dist)
            result=interpolateSpatialData(inter0, inter1,t*2 - 0.5, simTime0, simTime1)
        else:
            inter1=SpatialData(s1.cosmoPos, dist)
            result=interpolateSpatialData(inter1, s1, t*4 - 3, simTime1, simTime1)
        longanimation = True
        return result

    if s0.bodyName != '' and s1.bodyName != '' and s0.bodyName != s1.bodyName :
        longanimation = True
        bn = VIRUP.getClosestCommonAncestorName(s0.bodyName, s1.bodyName)
        planetpos = VIRUP.interpolateCoordinates(s0.bodyName, s1.bodyName, t)

        start=VIRUP.getCelestialBodyPosition(s0.bodyName, bn, simTime0)
        end=VIRUP.getCelestialBodyPosition(s1.bodyName, bn, simTime1)

        dist = (end-start).length()
        maxscale = min(s1.scale, 1.0 / (dist))
        if t <= 0.25:
            scale = 1.0 / interpolateLog(s0.scale, maxscale, 4*t)
            planetpos = start
        elif t <= 0.75:
            scale = 1.0 / maxscale
            # maybe try some smoother t
            tprime = t*2 - 0.5
            planetpos = interpolateLinear(start, end, 2*t - 0.5)
        else:
            scale = 1.0 / interpolateLog(maxscale, s1.scale, 4*t - 3)
            planetpos = end
    else:
        bn = s1.bodyName
        if bn == '':
            bn = s0.bodyName

    return SpatialData(
        interpolateLinear(s0.cosmoPos, s1.cosmoPos, t),
        scale,
        bn,
        s1.systemName,
        planetpos
    )

def interpolateTemporalData(t0, t1, t):
    return TemporalData(
        interpolateLog(t0.timeCoeff, t1.timeCoeff, t),
        interpolateDateTime(t0.simulationTime, t1.simulationTime, t)
    )

def interpolateUI(ui0, ui1, t):
    return UI(
        interpolateLinear(ui0.sdsslum, ui1.sdsslum, t),
        interpolateLinear(ui0.gaialum, ui1.gaialum, t),
        interpolateLinear(ui0.illustrislum, ui1.illustrislum, t),
        interpolateLinear(ui0.agoralum, ui1.agoralum, t),
    )

def interpolateScene(sc0, sc1, t):
    simTime0=interpolateDateTime(sc0.temporalData.simulationTime, sc1.temporalData.simulationTime, 0.25)
    simTime1=interpolateDateTime(sc0.temporalData.simulationTime, sc1.temporalData.simulationTime, 0.75)
    return Scene(
        interpolateSpatialData(sc0.spatialData, sc1.spatialData, t, simTime0, simTime1),
        interpolateTemporalData(sc0.temporalData, sc1.temporalData, t),
        interpolateUI(sc0.ui, sc1.ui, t)
    )

solareclipsedt = QDateTime(QDate(2020, 7, 13), QTime(7, 55, 46))

scenes = [
    # International Space Station
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 100, 'ISS', 'Solar System'),
          TemporalData(1.0), UI(0.0, 0.0, 0.0, 0.0)),
    # Earth-Moon dynamics
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 7500000, 'Earth', 'Solar System'),
          TemporalData(1.0, solareclipsedt), UI(0.0, 1.0, 0.0, 0.0)),
    # Milky Way
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 0.78129*3.75e+20),
           TemporalData(), UI(0.0, 0.0, 0.015, 1.0)),
    # Illustris-base
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 2.5e+22),
           TemporalData(), UI(0.0, 0.0, 1.0, 0.0)),
    # Illustris
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1.875e+24),
           TemporalData(), UI(0.0, 0.0, 1.0, 0.0)),
    # Whole cube
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 2.1e+25),
           TemporalData(), UI(1.0, 0.0, 0.0, 0.0)),
]

id = 3
disableanimations = False
personheight=1.5
angle=60*3.1415/180

def getCosmoShift():
    global angle
    shiftval = personheight*3.24078e-20 / VIRUP.scale
    try:
        VRHandler
    except NameError:
        return Vector3(cos(angle) * shiftval, sin(angle) * shiftval, 0.05 * shiftval)
    else:
        if VRHandler.drivername != "OpenVR":
            return Vector3(cos(angle) * shiftval, sin(angle) * shiftval, 0.05 * shiftval)
        else:
            return Vector3(0, 0, -shiftval)

def getPlanetShift():
    global angle
    shiftval = personheight / VIRUP.scale
    try:
        VRHandler
    except NameError:
        return Vector3(cos(angle) * shiftval, sin(angle) * shiftval, 0.05 * shiftval)
    else:
        if VRHandler.drivername != "OpenVR":
            return Vector3(cos(angle) * shiftval, sin(angle) * shiftval, 0.05 * shiftval)
        else:
            return Vector3(0, 0, -shiftval)

def setSceneId(newid):
    global timer
    global id
    global currentscene
    global animationduration

    if newid > id:
        if id < 2:
            animationduration = 20
        elif id == 2:
            animationduration = 30
        elif id == 3:
            animationduration = 90
        elif id == 4:
            animationduration = 300
    else:
        if id < 3:
            animationduration = 20
        elif id == 3:
            animationduration = 30
        elif id == 4:
            animationduration = 90
        elif id == 5:
            animationduration = 300

    oldid = id
    id = newid
    timer.restart()
    if disableanimations:
        currentscene = scenes[id]
    else:
        currentscene=Scene(SpatialData(VIRUP.cosmoPosition - getCosmoShift(), 1.0 / VIRUP.scale, VIRUP.planetTarget, VIRUP.planetarySystemName),
           TemporalData(VIRUP.timeCoeff, VIRUP.simulationTime), scenes[oldid].ui)
        if scenes[id].spatialData.systemName == currentscene.spatialData.systemName and (scenes[id].spatialData.cosmoPos - currentscene.spatialData.cosmoPos).length() > 0.1:
            currentscene.spatialData.systemName = ""

def toggleAnimations():
    global disableanimations
    disableanimations = not disableanimations

def keyPressEvent(e):
    global disableanimations

    # if spacebar pressed, start animation
    numpad_mod = int(e.modifiers()) == Qt.KeypadModifier
    if e.key() == Qt.Key_0 and numpad_mod:
        setSceneId(0)
    elif e.key() == Qt.Key_1 and numpad_mod:
        setSceneId(1)
    elif e.key() == Qt.Key_2 and numpad_mod:
        setSceneId(2)
    elif e.key() == Qt.Key_3 and numpad_mod:
        setSceneId(3)
    elif e.key() == Qt.Key_4 and numpad_mod:
        setSceneId(4)
    elif e.key() == Qt.Key_5 and numpad_mod:
        setSceneId(5)
    elif e.key() == Qt.Key_6 and numpad_mod:
        setSceneId(6)
    elif e.key() == Qt.Key_7 and numpad_mod:
        setSceneId(7)
    elif e.key() == Qt.Key_8 and numpad_mod:
        setSceneId(8)
    elif e.key() == Qt.Key_9 and numpad_mod:
        setSceneId(9)
    elif e.key() == Qt.Key_Minus and numpad_mod:
        toggleAnimations()
    elif e.key() == Qt.Key_Space:
        setSceneId(-1)
    else:
        return


camrotationrate=0
def setCamRotationRate(rate):
    global camrotationrate
    global timerCam
    camrotationrate=rate
    timerCam.restart()


def initScene():
    global angle
    global timer
    global timerCam
    global longanimation
    global currentscene

    VIRUP.simulationTime = solareclipsedt

    timer = QElapsedTimer()
    timerCam = QElapsedTimer()
    timerCam.start()
    longanimation = False
    currentscene = None
    VIRUP.camPitch=0
    VIRUP.camYaw=angle

animationduration=10
ascending = True
def updateScene():
    global id
    global ascending
    global angle
    global timer
    global camrotationrate
    global timerCam
    global longanimation
    global currentscene

    if camrotationrate != 0:
        VIRUP.camYaw+=camrotationrate * timerCam.restart() / 1000.0

    if id not in range(len(scenes)) or not VIRUP.isServer:
        return

    if id >= 3 and camrotationrate != 0.05:
        setCamRotationRate(0.008)
    elif id < 3 and camrotationrate != 0.0:
        setCamRotationRate(0.0)
    if id < 2:
        angle = 60*3.1415/180.0
        VIRUP.camYaw = angle
    elif id > 2:
        angle = 0.0


    if longanimation:
        t = timer.elapsed() / (animationduration * 1500.0)
    else:
        t = timer.elapsed() / (animationduration * 1000.0)
    t = smoothstep(t)
    nextid = -1
    if timer.isValid() and t < 1.0 and t >= 0.0 and currentscene != None:
        if id == 2:
            angle = (1.0-t)*60*3.1415/180.0
            VIRUP.camYaw = angle
        scene=interpolateScene(currentscene, scenes[id], t)
    else:
        if id == 2:
            angle = 0.0
            VIRUP.camYaw = angle
        timer.invalidate()
        scene=scenes[id]
        if id != -1:
            if ascending and id == 5:
                ascending = False
            if not ascending and id == 3:
                ascending = True
            if ascending:
                nextid = id+1
            else:
                nextid = id-1

    spatialData = scene.spatialData
    VIRUP.scale = spatialData.scale
    if spatialData.systemName != '':
        VIRUP.planetarySystemName = spatialData.systemName

    if spatialData.bodyName != '' and VIRUP.planetarySystemLoaded:
        VIRUP.planetTarget = spatialData.bodyName
        VIRUP.planetPosition = spatialData.planetPos + getPlanetShift()
    else:
        VIRUP.cosmoPosition = spatialData.cosmoPos + getCosmoShift()

    temporalData = scene.temporalData
    VIRUP.timeCoeff = temporalData.timeCoeff
    if temporalData.simulationTime != None:
        if temporalData.simulationTime.isValid() and t <= 1:
            VIRUP.simulationTime = temporalData.simulationTime

    ui = scene.ui
    Universe.setVisibility("SDSS", ui.sdsslum)
    Universe.setVisibility("Gaia", ui.gaialum)
    Universe.setVisibility("Volumetric AGORA", ui.agoralum)
    p = 1
    if (ascending and id == 5) or (not ascending and id == 4):
        p = 15
    Universe.setVisibility("IllustrisTNG 0", ui.illustrislum**p)
    Universe.setVisibility("IllustrisTNG 1", ui.illustrislum**p)
    Universe.setVisibility("IllustrisTNG 2", ui.illustrislum**p)
    Universe.setVisibility("IllustrisTNG 3", ui.illustrislum**p)
    Universe.setVisibility("IllustrisTNG 4", ui.illustrislum**p)
    Universe.setVisibility("IllustrisTNG 5", ui.illustrislum**p)
    VIRUP.darkmatterEnabled = True

    if nextid != -1:
        setSceneId(nextid)

