from PythonQt.QtGui import QKeyEvent
from PythonQt.QtCore import QElapsedTimer
from PythonQt.QtCore import Qt
from PythonQt.QtCore import QDateTime
from PythonQt.QtCore import QDate
from PythonQt.QtCore import QTime
from PythonQt.QtCore import QTimeZone
from PythonQt.QtMultimedia import QSound
from PythonQt.libplanet import Vector3
from math import exp, log, isnan, atan2, asin, cos, sin, pi

def smoothstep(t, v0 = 0, v1 = 0):
    if t < 0.0:
        return 0.0
    if t > 1.0:
        return 1.00001
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
    def __init__(self, lumsdic=[]):
        self.lumsdic=lumsdic
    def getLum(self, name):
        if name in self.lumsdic.keys():
            return self.lumsdic[name]
        return 0.0

class Scene:
    def __init__(self, spatialData, temporalData = TemporalData(), ui = UI(), transitiontimeto=10.0, name="", custom=lambda *args: None):
        self.spatialData = spatialData
        self.temporalData = temporalData
        self.ui = ui
        self.name = name
        self.transitiontimeto = transitiontimeto
        self.custom = custom


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
    dic={}
    for label in Universe.getUniverseElementsNames():
        dic[label] = interpolateLinear(ui0.getLum(label), ui1.getLum(label), t)

    additional=["Exoplanets", "Constellations", "Orbits", "PlanetsLabels", "Debris", "AnimationTime"]
    for label in additional:
        dic[label] = interpolateLinear(ui0.getLum(label), ui1.getLum(label), t)

    return UI(dic)

def interpolateScene(sc0, sc1, t):
    simTime0=interpolateDateTime(sc0.temporalData.simulationTime, sc1.temporalData.simulationTime, 0.25)
    simTime1=interpolateDateTime(sc0.temporalData.simulationTime, sc1.temporalData.simulationTime, 0.75)
    return Scene(
        interpolateSpatialData(sc0.spatialData, sc1.spatialData, t, simTime0, simTime1),
        interpolateTemporalData(sc0.temporalData, sc1.temporalData, t),
        interpolateUI(sc0.ui, sc1.ui, t)
    )

#2021-06-26T01:52:07Z
#2021-06-25T22:00:07Z
solareclipsedt = QDateTime(QDate(2021, 6, 26), QTime(1, 51, 30), QTimeZone(0))

# CUSTOM
def clamp(val, minVal, maxVal):
    if val < minVal:
        return minVal
    elif val > maxVal:
        return maxVal
    return val

def squeeze_t(t, power):
    t = clamp(t, 0.0, 1.0)
    if t < 0.5:
        t *= 2.0
        t = t**power
        t /= 2.0
    if t > 0.5:
        t -= 0.5
        t *= 2.0
        t = 1.0 - t
        t = t**power
        t = 1.0 - t
        t /= 2.0
        t += 0.5

    return t

def fade_in_factor(t_harsh):
    t=squeeze_t(t_harsh,0.4)
    t=smoothstep(t)
    if t > 0.5:
        return 1.0
    return t*2

def fade_out_factor(t_harsh):
    t=squeeze_t(t_harsh,0.4)
    t=smoothstep(t)
    if t < 0.5:
        return 1.0
    return 1.0 - (2.0*(t-0.5))

def fade_out(t, t_harsh):
    global fade_factor
    fade_factor = fade_out_factor(t_harsh)

def intro(t, t_harsh):
    global fade_factor
    global id
    global scenes
    fade_factor = fade_in_factor(t_harsh) * fade_out_factor(t_harsh)
    scale0=scenes[id-1].spatialData.scale
    scale1=scenes[id].spatialData.scale
    VIRUP.scale=interpolateLog(scale0, scale1, t_harsh)

def black(t, t_harsh):
    global fade_factor
    fade_factor = 0.0

def begin(t, t_harsh):
    global fade_factor
    fade_factor = fade_in_factor(t_harsh)

def end(t, t_harsh):
    global fade_factor
    fade_factor = fade_out_factor(t_harsh)

def showOrbitsWhileTraveling(t, t_harsh):
    if t_harsh > 0.3 and t_harsh < 0.7:
        Universe.setVisibility("Orbits", 1.0)
        Universe.setVisibility("PlanetsLabels", 1.0)
    elif t_harsh <= 0.3:
        Universe.setVisibility("Orbits", (t_harsh*3)**5)
        Universe.setVisibility("PlanetsLabels", (t_harsh*3)**5)
    elif t_harsh >= 0.7:
        Universe.setVisibility("Orbits", (1.0 - (t_harsh-0.7)*3)**5)
        Universe.setVisibility("PlanetsLabels", (1.0 - (t_harsh-0.7)*3)**5)
# END CUSTOM

#sdsslum, gaialum, illustrislum, agoralum, hyg, exoplanets, constellations, orbits, labels, cmb):
scenes = [
# Intro
    # Earth
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'ISS', 'Solar System', Vector3(-50, 0, 30)),
          TemporalData(1.0), UI({"Hipparcos":1.0}), 1.0, "begin", begin),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'ISS', 'Solar System', Vector3(-50, 0, 30)),
          TemporalData(1.0), UI({"Hipparcos":1.0}), 3.0),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'ISS', 'Solar System', Vector3(-50, 0, 30)),
          TemporalData(1.0), UI({"Hipparcos":1.0, "Orbits":1.0, "PlanetsLabels":1.0}), 1.0),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'ISS', 'Solar System', Vector3(-50, 0, 30)),
          TemporalData(1.0), UI({"Hipparcos":1.0, "Orbits":1.0, "PlanetsLabels":1.0}), 3.0),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'ISS', 'Solar System', Vector3(-50, 0, 30)),
          TemporalData(1.0), UI({"Volumetric AGORA":1.0}), 1.0),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'ISS', 'Solar System', Vector3(-50, 0, 30)),
          TemporalData(1.0), UI({"Volumetric AGORA":1.0}), 3.0),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'ISS', 'Solar System', Vector3(-50, 0, 30)),
          TemporalData(1.0), UI({"IllustrisTNG":1.0}), 1.0),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'ISS', 'Solar System', Vector3(-50, 0, 30)),
          TemporalData(1.0), UI({"IllustrisTNG":1.0}), 3.0),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'ISS', 'Solar System', Vector3(-50, 0, 30)),
          TemporalData(1.0), UI({"SDSS":1.0}), 1.0),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'ISS', 'Solar System', Vector3(-50, 0, 30)),
          TemporalData(1.0), UI({"SDSS":1.0}), 3.0),




    # International Space Station Real scale
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'ISS', 'Solar System', Vector3(-50, 0, 30)),
          TemporalData(1.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 1.0),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'ISS', 'Solar System', Vector3(-50, 0, 30)),
          TemporalData(1.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 19.0),
    # Earth
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 15000000, 'Earth', 'Solar System'),
          TemporalData(1.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 10.0),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 15000000, 'Earth', 'Solar System'),
          TemporalData(1.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 20.0),
    # Moon
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 4000000, 'Moon', 'Solar System'),
          TemporalData(1.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 10.0),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 4000000, 'Moon', 'Solar System'),
          TemporalData(1.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 20.0),
    # Phobos
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 30000, 'Phobos', 'Solar System'),
          TemporalData(100.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 10.0, "phobos", showOrbitsWhileTraveling),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 30000, 'Phobos', 'Solar System'),
          TemporalData(500.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 20.0),
    # Solar System dynamics Constellations
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 5.65181e+12, 'Sun', 'Solar System'),
          TemporalData(10000000.0), UI({"Gaia":1.0, "Hipparcos":1.0, "Exoplanets":1.0, "Constellations":1.0, "Orbits":1.0, "PlanetsLabels":1.0}), 10.0),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 5.65181e+12, 'Sun', 'Solar System'),
          TemporalData(10000000.0), UI({"Gaia":1.0, "Hipparcos":1.0, "Exoplanets":1.0, "Constellations":1.0, "Orbits":1.0, "PlanetsLabels":1.0}), 20.0),
    # Milky Way
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 6.171e+20),
          TemporalData(), UI({"Andromeda":2.0, "M33":2.0, "LG Dwarves":5.0, "Exoplanets":0.1, "Volumetric AGORA":1.0, "Orbits":1.0, "Labels":1.0}), 10.0),
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 6.171e+20),
          TemporalData(), UI({"Andromeda":2.0, "M33":2.0, "LG Dwarves":5.0, "Exoplanets":0.1, "Volumetric AGORA":1.0, "Orbits":1.0, "Labels":1.0}), 20.0),
    # Local Group
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 3.04e+22),
          TemporalData(), UI({"Volumetric AGORA":1.0, "Andromeda":2.0, "M33":2.0, "LG Dwarves":5.0, "Labels2":1.0}), 10.0),
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 3.04e+22),
          TemporalData(), UI({"Volumetric AGORA":1.0, "Andromeda":2.0, "M33":2.0, "LG Dwarves":5.0, "Labels2":1.0}), 20.0),
    # Illustris
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 0.2e+25),
          TemporalData(), UI({"IllustrisTNG":1.0}), 10.0),
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 0.2e+25),
          TemporalData(), UI({"IllustrisTNG":1.0}), 20.0),
    # SDSS distant
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 2.0e+26),
          TemporalData(), UI({"SDSS":1.0}), 10.0),
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 2.0e+26),
          TemporalData(), UI({"SDSS":1.0}), 35.0),
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 2.0e+26),
          TemporalData(), UI({"SDSS":1.0}), 1.0, "end", end),

    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 15000000, 'Earth', 'Solar System'),
          TemporalData(1.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 1.0, "fooend", black)
]

def getIdFromName(name):
    global scenes

    for i in range(len(scenes)):
        if scenes[i].name == name:
            return i

id = 0
disableanimations = False
personheight=1.5
ToneMappingModel.exposure=0.3
shiftangle=0.0
shiftvertangle=0.0
fade_factor=1.0

def getCosmoShift():
    global shiftangle
    global shiftvertangle
    val=personheight*3.24078e-20 / VIRUP.scale
    try:
        VRHandler
    except NameError:
        return Vector3(cos(shiftvertangle)*cos(shiftangle)*val, cos(shiftvertangle)*sin(shiftangle)*val, sin(shiftvertangle) * val)
    else:
        if VRHandler.drivername != "OpenVR":
            return Vector3(cos(shiftvertangle)*cos(shiftangle)*val, cos(shiftvertangle)*sin(shiftangle)*val, sin(shiftvertangle)*0.05 * val)
        else:
            return Vector3(0, 0, -val)

def getPlanetShift():
    global shiftangle
    global shiftvertangle
    val=personheight / VIRUP.scale
    try:
        VRHandler
    except NameError:
        return Vector3(cos(shiftvertangle)*cos(shiftangle)*val, cos(shiftvertangle)*sin(shiftangle)*val, sin(shiftvertangle) * val)
    else:
        if VRHandler.drivername != "OpenVR":
            return Vector3(cos(shiftvertangle)*cos(shiftangle)*val, cos(shiftvertangle)*sin(shiftangle)*val, sin(shiftvertangle)*0.05 * val)
        else:
            return Vector3(0, 0, -val)

def setSceneId(newid):
    global timer
    global id
    global currentscene

    oldid = id
    id = newid
    if id not in range(len(scenes)):
        return
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


s=QSound(VIRUP.getVoiceoverPath())
def initScene():
    global timer
    global longanimation
    global currentscene
    global s

    VIRUP.simulationTime = solareclipsedt

    timer = QElapsedTimer()
    longanimation = False
    currentscene = None

    totaltime=0
    for scene in scenes:
        totaltime += scene.transitiontimeto
    print(totaltime / 60.0)
    print(totaltime)

    s.play()


def updateScene():
    global id
    global timer
    global longanimation
    global currentscene
    global shiftangle
    global shiftvertangle
    global fade_factor
    if id not in range(len(scenes)) or not VIRUP.isServer:
        return

    t_harsh = timer.elapsed() / ((scenes[id].transitiontimeto) * 1000.0)
    t = t_harsh
    nextid = -1
    if t <= 1.0 and t >= 0.0 and currentscene != None:
        scene=interpolateScene(currentscene, scenes[id], t)
    else:
        timer.invalidate()
        t_harsh = 1.0
        t = 1.0
        if currentscene != None:
            scene=interpolateScene(currentscene, scenes[id], 1.0)
        else:
            scene=scenes[id]
        nextid = id+1

    spatialData = scene.spatialData
    VIRUP.scale = spatialData.scale
    if spatialData.systemName != '':
        VIRUP.planetarySystemName = spatialData.systemName

    if spatialData.bodyName != '' and VIRUP.planetarySystemLoaded:
        VIRUP.planetTarget = spatialData.bodyName
        VIRUP.planetPosition = spatialData.planetPos
    else:
        VIRUP.cosmoPosition = spatialData.cosmoPos

    temporalData = scene.temporalData
    VIRUP.timeCoeff = temporalData.timeCoeff
    if temporalData.simulationTime != None:
        if temporalData.simulationTime.isValid() and t <= 1:
            VIRUP.simulationTime = temporalData.simulationTime

    ui = scene.ui
    for label in Universe.getUniverseElementsNames():
        if fade_factor != 0.0:
            if ToneMappingModel.exposure != 0.0:
                Universe.setVisibility(label, ui.getLum(label) * 0.3 * fade_factor / ToneMappingModel.exposure)
            else:
                Universe.setVisibility(label, ui.getLum(label) * fade_factor)

    additional=["Exoplanets", "Constellations", "Orbits", "PlanetsLabels"]
    for label in additional:
        Universe.setVisibility(label, ui.getLum(label))

    ToneMappingModel.exposure = 0.3
    fade_factor = 1.0
    Universe.setLabelsOrbitsOnly([])

    animationtime=ui.getLum("AnimationTime")
    Universe.setAnimationTime(animationtime)

    VIRUP.camYaw = shiftangle
    VIRUP.camPitch = -shiftvertangle
    shiftangle = 0.0
    shiftvertangle = 0.05

    # apply custom function
    scenes[id].custom(t, t_harsh)
    ToneMappingModel.exposure *= fade_factor

    if nextid != -1 and nextid < len(scenes):
        setSceneId(nextid)

    if spatialData.bodyName != '' and VIRUP.planetarySystemLoaded:
        VIRUP.planetPosition += getPlanetShift()
    else:
        VIRUP.cosmoPosition += getCosmoShift()
