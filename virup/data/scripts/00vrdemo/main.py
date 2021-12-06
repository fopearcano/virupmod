from PythonQt.QtGui import QKeyEvent, Qt, QDateTime, QDate, QTime, QTimeZone
from PythonQt.QtMultimedia import QSound
from PythonQt.libplanet import Vector3
from PythonQt.virup import Transition, Scene, SceneSpatialData, SceneTemporalData, SceneUI
from math import cos, sin

# CUSTOM
def smoothstep(t, v0 = 0, v1 = 0):
    if t < 0.0:
        return 0.0
    if t > 1.0:
        return 1.00001
    return (6*t**5 - 15*t**4 + 10*t**3 + 0.5*(v1-v0)*t**2 + v0*t) / (1.0 + 0.5*(v1+v0))

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

#2021-06-26T01:52:07Z
#2021-06-25T22:00:07Z
solareclipsedt = QDateTime(QDate(2021, 6, 26), QTime(1, 51, 30), QTimeZone(0))

transitions = [
# Intro
    # Earth
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 1, Vector3(-50, 0, 30)),
          SceneTemporalData(1.0), SceneUI({"Hipparcos":1.0})), 1.0, "begin", "begin"),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 1, Vector3(-50, 0, 30)),
          SceneTemporalData(1.0), SceneUI({"Hipparcos":1.0})), 3.0),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 1, Vector3(-50, 0, 30)),
          SceneTemporalData(1.0), SceneUI({"Hipparcos":1.0, "Orbits":1.0, "PlanetsLabels":1.0})), 1.0),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 1, Vector3(-50, 0, 30)),
          SceneTemporalData(1.0), SceneUI({"Hipparcos":1.0, "Orbits":1.0, "PlanetsLabels":1.0})), 3.0),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 1, Vector3(-50, 0, 30)),
          SceneTemporalData(1.0), SceneUI({"Volumetric AGORA":1.0})), 1.0),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 1, Vector3(-50, 0, 30)),
          SceneTemporalData(1.0), SceneUI({"Volumetric AGORA":1.0})), 3.0),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 1, Vector3(-50, 0, 30)),
          SceneTemporalData(1.0), SceneUI({"IllustrisTNG":1.0})), 1.0),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 1, Vector3(-50, 0, 30)),
          SceneTemporalData(1.0), SceneUI({"IllustrisTNG":1.0})), 3.0),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 1, Vector3(-50, 0, 30)),
          SceneTemporalData(1.0), SceneUI({"SDSS":1.0})), 1.0),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 1, Vector3(-50, 0, 30)),
          SceneTemporalData(1.0), SceneUI({"SDSS":1.0})), 3.0),



    # International Space Station Real scale
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 1, Vector3(-50, 0, 30)),
          SceneTemporalData(1.0), SceneUI({"Gaia":1.0, "Hipparcos":1.0})), 1.0),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 1, Vector3(-50, 0, 30)),
          SceneTemporalData(1.0), SceneUI({"Gaia":1.0, "Hipparcos":1.0})), 19.0),
    # Earth
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Earth', 15000000),
          SceneTemporalData(1.0), SceneUI({"Gaia":1.0, "Hipparcos":1.0})), 10.0),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Earth', 15000000),
          SceneTemporalData(1.0), SceneUI({"Gaia":1.0, "Hipparcos":1.0})), 20.0),
    # Moon
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Moon', 4000000),
          SceneTemporalData(1.0), SceneUI({"Gaia":1.0, "Hipparcos":1.0})), 10.0),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Moon', 4000000),
          SceneTemporalData(1.0), SceneUI({"Gaia":1.0, "Hipparcos":1.0})), 20.0),
    # Phobos
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Phobos', 30000),
          SceneTemporalData(100.0), SceneUI({"Gaia":1.0, "Hipparcos":1.0})), 10.0, "phobos", "showOrbitsWhileTraveling"),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Phobos', 30000),
          SceneTemporalData(500.0), SceneUI({"Gaia":1.0, "Hipparcos":1.0})), 20.0),
    # Solar System dynamics Constellations
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Sun', 5.65181e+12),
          SceneTemporalData(10000000.0), SceneUI({"Gaia":1.0, "Hipparcos":1.0, "Exoplanets":1.0, "Constellations":1.0, "Orbits":1.0, "PlanetsLabels":1.0})), 10.0),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Sun', 5.65181e+12),
          SceneTemporalData(10000000.0), SceneUI({"Gaia":1.0, "Hipparcos":1.0, "Exoplanets":1.0, "Constellations":1.0, "Orbits":1.0, "PlanetsLabels":1.0})), 20.0),
    # Milky Way
    Transition(Scene(SceneSpatialData(Universe, 6.171e+20, Vector3(-0.43, -8.24, -0.81)),
          SceneTemporalData(), SceneUI({"Andromeda":2.0, "M33":2.0, "LG Dwarves":5.0, "Exoplanets":0.1, "Volumetric AGORA":1.0, "Orbits":1.0, "Labels":1.0})), 10.0),
    Transition(Scene(SceneSpatialData(Universe, 6.171e+20, Vector3(-0.43, -8.24, -0.81)),
          SceneTemporalData(), SceneUI({"Andromeda":2.0, "M33":2.0, "LG Dwarves":5.0, "Exoplanets":0.1, "Volumetric AGORA":1.0, "Orbits":1.0, "Labels":1.0})), 20.0),
    # Local Group
    Transition(Scene(SceneSpatialData(Universe, 3.04e+22, Vector3(-0.43, -8.24, -0.81)),
          SceneTemporalData(), SceneUI({"Volumetric AGORA":1.0, "Andromeda":2.0, "M33":2.0, "LG Dwarves":5.0, "Labels2":1.0})), 10.0),
    Transition(Scene(SceneSpatialData(Universe, 3.04e+22, Vector3(-0.43, -8.24, -0.81)),
          SceneTemporalData(), SceneUI({"Volumetric AGORA":1.0, "Andromeda":2.0, "M33":2.0, "LG Dwarves":5.0, "Labels2":1.0})), 20.0),
    # Illustris
    Transition(Scene(SceneSpatialData(Universe, 0.2e+25, Vector3(-0.43, -8.24, -0.81)),
          SceneTemporalData(), SceneUI({"IllustrisTNG":1.0})), 10.0),
    Transition(Scene(SceneSpatialData(Universe, 0.2e+25, Vector3(-0.43, -8.24, -0.81)),
          SceneTemporalData(), SceneUI({"IllustrisTNG":1.0})), 20.0),
    # SDSS distant
    Transition(Scene(SceneSpatialData(Universe, 2.0e+26, Vector3(-0.43, -8.24, -0.81)),
          SceneTemporalData(), SceneUI({"SDSS":1.0})), 10.0),
    Transition(Scene(SceneSpatialData(Universe, 2.0e+26, Vector3(-0.43, -8.24, -0.81)),
          SceneTemporalData(), SceneUI({"SDSS":1.0})), 35.0),
    Transition(Scene(SceneSpatialData(Universe, 2.0e+26, Vector3(-0.43, -8.24, -0.81)),
          SceneTemporalData(), SceneUI({"SDSS":1.0})), 1.0, "end", "end"),

    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Earth', 15000000),
          SceneTemporalData(1.0), SceneUI({"Gaia":1.0, "Hipparcos":1.0})), 1.0, "fooend", "black")
]

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
    val=personheight*3.24078e-20 / Universe.scale
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
    val=personheight / Universe.scale
    try:
        VRHandler
    except NameError:
        return Vector3(cos(shiftvertangle)*cos(shiftangle)*val, cos(shiftvertangle)*sin(shiftangle)*val, sin(shiftvertangle) * val)
    else:
        if VRHandler.drivername != "OpenVR":
            return Vector3(cos(shiftvertangle)*cos(shiftangle)*val, cos(shiftvertangle)*sin(shiftangle)*val, sin(shiftvertangle)*0.05 * val)
        else:
            return Vector3(0, 0, -val)

def setTransitionId(newid):
    global id
    global currentscene

    oldid = id
    id = newid
    if oldid in range(len(transitions)):
        transitions[oldid].stop()
    if id not in range(len(transitions)):
        return
    transitions[id].play()
    if disableanimations:
        currentscene = transitions[id].getDestination()
    else:
        if oldid == -1:
            currentscene=Scene.getCurrentState(Universe)
            sd = currentscene.getSpatialData()
            if sd.getSystemName() == "" and sd.getBodyName() == "":
                sd.setPosition(sd.getPosition() - getCosmoShift())
                currentscene.setSpatialData(sd)
            else:
                sd.setPosition(sd.getPosition() - getPlanetShift())
                currentscene.setSpatialData(sd)

        else:
            currentscene=transitions[oldid].getDestination()

autoIdScrolling=True
def stop():
    global autoIdScrolling
    global s
    autoIdScrolling=False
    setTransitionId(-1)
    del s

def toggleAnimations():
    global disableanimations
    disableanimations = not disableanimations

def keyPressEvent(e):
    global disableanimations
    global id

    # if spacebar pressed, start animation
    numpad_mod = int(e.modifiers()) == Qt.KeypadModifier
    if e.key() == Qt.Key_0 and numpad_mod:
        setTransitionId(0)
    elif e.key() == Qt.Key_1 and numpad_mod:
        setTransitionId(1)
    elif e.key() == Qt.Key_2 and numpad_mod:
        setTransitionId(2)
    elif e.key() == Qt.Key_3 and numpad_mod:
        setTransitionId(3)
    elif e.key() == Qt.Key_4 and numpad_mod:
        setTransitionId(4)
    elif e.key() == Qt.Key_5 and numpad_mod:
        setTransitionId(5)
    elif e.key() == Qt.Key_6 and numpad_mod:
        setTransitionId(6)
    elif e.key() == Qt.Key_7 and numpad_mod:
        setTransitionId(7)
    elif e.key() == Qt.Key_8 and numpad_mod:
        setTransitionId(8)
    elif e.key() == Qt.Key_9 and numpad_mod:
        setTransitionId(9)
    elif e.key() == Qt.Key_Minus and numpad_mod:
        toggleAnimations()
    elif e.key() == Qt.Key_Space:
        stop()
    elif e.key() == Qt.Key_Enter:
        setTransitionId((id+1) % len(transitions))
    else:
        return


s=QSound(VIRUP.getVoiceoverPath())
def initScene():
    global currentscene
    global s

    Universe.simulationTime = solareclipsedt

    currentscene = None
    setTransitionId(0)

    totaltime=0
    for t in transitions:
        totaltime += t.getDuration()
    print(totaltime / 60.0)
    print(totaltime)

    s.play()


def updateScene():
    global id
    global currentscene
    global shiftangle
    global shiftvertangle
    global fade_factor
    global autoIdScrolling
    if id not in range(len(transitions)) or not VIRUP.isServer:
        return

    nextid = -1
    if not transitions[id].updateUniverse(Universe, currentscene, fade_factor, getCosmoShift(), getPlanetShift()):
        nextid = id+1

    ToneMappingModel.exposure = 0.3
    fade_factor = 1.0
    Universe.setLabelsOrbitsOnly([])

    VIRUP.camYaw = shiftangle
    VIRUP.camPitch = -shiftvertangle
    shiftangle = 0.0
    shiftvertangle = 0.05

    ToneMappingModel.exposure *= fade_factor

    if nextid != -1 and nextid < len(transitions) and autoIdScrolling:
        setTransitionId(nextid)

