from PythonQt.QtGui import QKeyEvent
from PythonQt.QtCore import QElapsedTimer
from PythonQt.QtCore import Qt
from PythonQt.QtCore import QDateTime
from PythonQt.QtCore import QDate
from PythonQt.QtCore import QTime
from PythonQt.libplanet import Vector3
from math import exp
from math import log



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
    def __init__(self, sdsslum=0.0, gaialum=0.0, illustrislum=0.0, agoralum=0.0, hyg=0.0, exoplanets=0.0, constellations=0.0, orbits=0.0, labels=0.0):
        self.sdsslum = sdsslum 
        self.gaialum = gaialum 
        self.illustrislum = illustrislum 
        self.agoralum = agoralum
        self.hyg=hyg
        self.exoplanets = exoplanets
        self.constellations = constellations
        self.orbits = orbits
        self.labels = labels

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
    planetpos = interpolateLinear(s0.planetPos, s1.planetPos, t)

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
        if dist == 0:
            print("DIST == 0")
            print(s0.bodyName)
            print(start)
            print(s1.bodyName)
            print(end)
            print(bn)
            maxscale = s1.scale
        else:
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
        interpolateLinear(ui0.hyg, ui1.hyg, t),
        interpolateLinear(ui0.exoplanets, ui1.exoplanets, t),
        interpolateLinear(ui0.constellations, ui1.constellations, t),
        interpolateLinear(ui0.orbits, ui1.orbits, t),
        interpolateLinear(ui0.labels, ui1.labels, t),
    )

def interpolateScene(sc0, sc1, t):
    simTime0=interpolateDateTime(sc0.temporalData.simulationTime, sc1.temporalData.simulationTime, 0.25)
    simTime1=interpolateDateTime(sc0.temporalData.simulationTime, sc1.temporalData.simulationTime, 0.75)
    return Scene(
        interpolateSpatialData(sc0.spatialData, sc1.spatialData, t, simTime0, simTime1),
        interpolateTemporalData(sc0.temporalData, sc1.temporalData, t),
        interpolateUI(sc0.ui, sc1.ui, t)
    )

solareclipsedt = QDateTime(QDate(2021, 6, 16), QTime(11, 20, 00))

#sdsslum, gaialum, illustrislum, agoralum, hyg, exoplanets, constellations, orbits, labels):
scenes = [
    # International Space Station Real scale
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'ISS', 'Solar System', Vector3(-50, 0, 30)),
          TemporalData(1.0), UI(0.0, 0.0, 0.0, 0.0, 10000.0, 0.0, 0.0, 0.0, 0.0)),
    # International Space Station
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 100, 'ISS', 'Solar System', Vector3(-50, 0, 30)),
          TemporalData(1.0), UI(0.0, 0.0, 0.0, 0.0, 10000.0, 0.0, 0.0, 0.0, 0.0)),
    # Earth-Moon dynamics
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 15000000, 'Earth', 'Solar System'),
          TemporalData(1.0), UI(0.0, 0.0, 0.0, 0.0, 10000.0, 0.0, 0.0, 0.0, 0.0)),
    # Earth-Moon dynamics
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 350000000, 'Earth', 'Solar System'),
          TemporalData(1.0), UI(0.0, 0.0, 0.0, 0.0, 10000.0, 0.0, 0.0, 0.0, 0.0)),
    # Phobos
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 30000, 'Phobos', 'Solar System'),
          TemporalData(500.0), UI(0.0, 0.0, 0.0, 0.0, 10000.0, 0.0, 0.0, 0.0, 0.0)),
    # Saturn moons dynamics
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 2000000000, 'Saturn', 'Solar System'),
          TemporalData(100000.0), UI(0.0, 0.0, 0.0, 0.0, 10000.0, 0.0, 0.0, 1.0, 1.0)),
    # Enceladus
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 3000000, 'Rhea',  'Solar System'),
          TemporalData(1.0), UI(0.0, 0.0, 0.0, 0.0, 10000.0, 0.0, 0.0, 0.0, 0.0)),
    # Solar System dynamics Constellations
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 5.65181e+12, 'Sun', 'Solar System'),
          TemporalData(10000000.0), UI(0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0)),
    # Kepler-11 dynamics
    Scene(SpatialData(Vector3(0.20798, -0.209748, 0.537371), 1.65181e+11, 'Kepler-11', 'Kepler-11'),
          TemporalData(10000.0), UI(0.0, 0.0, 0.0, 0.0, 10000.0, 1.0, 1.0, 1.0, 1.0)),
    # 51 Peg dynamics
    Scene(SpatialData(Vector3(0.0132362, -0.00132559, 0.00625585), 1.65181e+10, '51 Peg', '51 Peg'),
          TemporalData(10000.0), UI(0.0, 0.0, 0.0, 0.0, 10000.0, 1.0, 1.0, 1.0, 1.0)),
    # Milky Way
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 6.171e+20),
           TemporalData(), UI(0.0, 0.0, 0.01, 1.0, 0.0, 0.1, 0.0, 0.0, 0.0)),
    # Illustris
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 0.2e+25),
           TemporalData(), UI(0.0, 0.0, 1.0, 0.0, 0.0, 0.0)),
    # SDSS close
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 6.0e+25),
           TemporalData(), UI(1.0, 0.0, 0.0, 0.0, 0.0, 0.0)),
    # SDSS distant
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 2.0e+27),
           TemporalData(), UI(1.0, 0.0, 0.0, 0.0, 0.0, 0.0)),
    # Gaia In
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 5.65181e+13),
          TemporalData(10000000.0), UI(0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0)),
    # Gaia Mid
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 4.0e+19),
          TemporalData(10000000.0), UI(0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0)),
    # Gaia Out
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1.7e+22),
          TemporalData(10000000.0), UI(0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0)),
]

id = 0
disableanimations = False
personheight=1.5
ToneMappingModel.exposure=0.3

def getCosmoShift():
    try:
        VRHandler
    except NameError:
        return Vector3(personheight*3.24078e-20 / VIRUP.scale, 0, 0.05 * personheight*3.24078e-20 / VIRUP.scale)
    else:
        if VRHandler.drivername != "OpenVR":
            return Vector3(personheight*3.24078e-20 / VIRUP.scale, 0, 0.05 * personheight*3.24078e-20 / VIRUP.scale)
        else:
            return Vector3(0, 0, -personheight*3.24078e-20 / VIRUP.scale)

def getPlanetShift():
    try:
        VRHandler
    except NameError:
        return Vector3(personheight / VIRUP.scale, 0, 0.05 * personheight / VIRUP.scale)
    else:
        if VRHandler.drivername != "OpenVR":
            return Vector3(personheight / VIRUP.scale, 0, 0.05 * personheight / VIRUP.scale)
        else:
            return Vector3(0, 0, -personheight / VIRUP.scale)

def setSceneId(newid):
    global timer
    global id
    global currentscene

    oldid = id
    id = newid
    timer.restart()
    if disableanimations:
        currentscene = scenes[id]
    else:
        if oldid == -1:
            currentscene=Scene(SpatialData(VIRUP.cosmoPosition - getCosmoShift(), 1.0 / VIRUP.scale, VIRUP.planetTarget, VIRUP.planetarySystemName),
               TemporalData(VIRUP.timeCoeff, VIRUP.simulationTime), scenes[oldid].ui)
        else:
            currentscene=scenes[oldid]
        if scenes[id].spatialData.systemName == currentscene.spatialData.systemName and (scenes[id].spatialData.cosmoPos - currentscene.spatialData.cosmoPos).length() > 0.1:
            currentscene.spatialData.systemName = ""

def toggleAnimations():
    global disableanimations
    disableanimations = not disableanimations

def keyPressEvent(e):
    global disableanimations
    global id

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
    elif e.key() == Qt.Key_Enter:
        setSceneId((id+1) % len(scenes))
    else:
        return


def initScene():
    global timer
    global longanimation
    global currentscene

    VIRUP.simulationTime = solareclipsedt

    timer = QElapsedTimer()
    longanimation = False
    currentscene = None

def updateScene():
    global id
    global timer
    global longanimation
    global currentscene
    if id not in range(len(scenes)) or not VIRUP.isServer:
        return

    if longanimation:
        t = timer.elapsed() / 15000.0
    else:
        t = timer.elapsed() / 10000.0
    if t <= 1.0 and t >= 0.0 and currentscene != None:
        scene=interpolateScene(currentscene, scenes[id], t)
    else:
        timer.invalidate()
        scene=scenes[id]

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
    if id == 5:
        p = 15
    Universe.setVisibility("IllustrisTNG", ui.illustrislum**p)
    """
    Universe.setVisibility("IllustrisTNG 0", ui.illustrislum**p)
    Universe.setVisibility("IllustrisTNG 1", ui.illustrislum**p)
    Universe.setVisibility("IllustrisTNG 2", ui.illustrislum**p)
    Universe.setVisibility("IllustrisTNG 3", ui.illustrislum**p)
    Universe.setVisibility("IllustrisTNG 4", ui.illustrislum**p)
    Universe.setVisibility("IllustrisTNG 5", ui.illustrislum**p)
    """
    Universe.setVisibility("Hipparcos", ui.hyg)
    Universe.setVisibility("Exoplanets", ui.exoplanets)
    Universe.setVisibility("Constellations", ui.constellations)
    if ui.orbits > 0.5:
        VIRUP.orbitsEnabled = 1.0
    else:
        VIRUP.orbitsEnabled = 0.0
    if ui.labels > 0.5:
        VIRUP.labelsEnabled = 1.0
    else:
        VIRUP.labelsEnabled = 0.0
    VIRUP.darkmatterEnabled = True

    if id >= 14:
        Universe.setVisibility("Labels", 1.0);
    else:
        Universe.setVisibility("Labels", 0.0);

