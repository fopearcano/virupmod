from PythonQt.QtCore import Qt, QDateTime, QDate, QTime, QTimeZone
from PythonQt.QtMultimedia import QSound
from PythonQt.libplanet import Vector3
from PythonQt.virup import Transition, Scene, SceneSpatialData, SceneTemporalData, SceneUI

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
    Animator.fadeFactor = 0.0

def forceHorizontalPiOver2(t, t_harsh):
    return
    Animator.shiftHorizontalAngle = 3.1415/2

def begin(t, t_harsh):
    forceHorizontalPiOver2(t, t_harsh)
    Animator.fadeFactor = fade_in_factor(t_harsh)

def end(t, t_harsh):
    forceDirectTransition(t, t_harsh)
    #Animator.fadeFactor = fade_out_factor(t_harsh)

def showOrbitsWhileTraveling(t, t_harsh):
    l=["Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"]
    Universe.setLabelsOrbitsOnly(l)
    if t_harsh > 0.3 and t_harsh < 0.7:
        Universe.setVisibility("Orbits", 1.0)
        Universe.setVisibility("PlanetsLabels", 1.0)
    elif t_harsh <= 0.3:
        Universe.setVisibility("Orbits", (t_harsh*3)**5)
        Universe.setVisibility("PlanetsLabels", (t_harsh*3)**5)
    elif t_harsh >= 0.7:
        Universe.setVisibility("Orbits", (1.0 - (t_harsh-0.7)*3)**5)
        Universe.setVisibility("PlanetsLabels", (1.0 - (t_harsh-0.7)*3)**5)

def forceDirectTransition(t, t_harsh):
    return
    SceneSpatialData.setForceDirectInterpolation(True)

def forceDirectTransitionAndAngle(t, t_harsh):
    return
    forceDirectTransition(t, t_harsh)
    forceHorizontalPiOver2(t, t_harsh)

def sdss(t, t_harsh):
    ToneMappingModel.contrast = 1.3 - t*0.3
    if t < 0.1:
        Universe.setVisibility("IllustrisTNG", 1.0 - (10*t))
    else:
        Universe.setVisibility("IllustrisTNG", 0.0)

def debris(t, t_harsh):
    forceDirectTransitionAndAngle(t, t_harsh)
    #Universe.setVisibility("Debris", 1.0)

def saturn1(t, t_harsh):
    forceHorizontalPiOver2(t, t_harsh)
    if t < 0.5:
        showOrbitsWhileTraveling(t, t_harsh)
    else:
        showOrbitsWhileTraveling(0.5, 0.5)
    l=["Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Mimas", "Enceladus", "Dione", "Tethys", "Rhea", "Hyperion", "Titan", "Iapetus"]
    Universe.setLabelsOrbitsOnly(l)
    ToneMappingModel.exposure=0.3+2.7*t
    Universe.setVisibility("Hipparcos", 0.1 * 0.3 / ToneMappingModel.exposure)

def mw(t, t_harsh):
    SceneSpatialData.setForceDirectInterpolation(True)

def wait(t, t_harsh):
    Animator.shiftHorizontalAngle = 20*t**0.7

def asteroids(t, t_harsh):
    forceDirectTransitionAndAngle(t, t_harsh)
    Universe.setVisibility("Asteroids", t**0.1)

def illustris(t, t_harsh):
    ToneMappingModel.contrast = 1.0 + t*0.3

# END CUSTOM

#2021-06-26T01:52:07Z
#2021-06-25T22:00:07Z
startdt = QDateTime(QDate(2022, 3, 19), QTime(10, 45, 00), QTimeZone(0))
isspos = Vector3(-48, 37, 20) # -50 0 30


transitions = [
# Intro
    # Earth
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Earth', 30000000),
          SceneTemporalData(1.0, startdt), SceneUI({"Hipparcos":0.1})), 10.0, "Earth", "forceDirectTransitionAndAngle", 0.0, 0.09),
    # Debris
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Earth', 40000000),
          SceneTemporalData(1000.0), SceneUI({"Gaia":1.0, "Hipparcos":0.1, "Debris":1.0})), 10.0, "Debris", "debris", 10000.0, 10000.0),
    # Saturn
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Saturn', 300000000),
       SceneTemporalData(1000.0), SceneUI({"Hipparcos":0.1, "Orbits":1.0, "PlanetsLabels":1.0})), 10.0, 'Saturn', 'saturn1'),
    #Solar System
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Sun', 1.65181e+12),
      SceneTemporalData(10000000.0), SceneUI({"Hipparcos":0.1, "Constellations":0.0, "Orbits":1.0, "PlanetsLabels":1.0, "Asteroids":1.0, "Constellations":1.0})), 10.0, "Solar System", "forceDirectTransitionAndAngle"),
    # Kepler-11
    Transition(Scene(SceneSpatialData(Universe, 'Kepler-11', 'Kepler-11', 1e11, Vector3(0.0, 0.0, 0.0)),
          SceneTemporalData(20000.0), SceneUI({"Hipparcos":0.1, "Constellations":0.0, "Orbits":1.0, "PlanetsLabels":1.0})), 10.0, "Kepler-11", "forceHorizontalPiOver2"),
    # Milky Way
    Transition(Scene(SceneSpatialData(Universe, 6.171e+20),#, Vector3(-0.43, -8.24, -0.81)),
          SceneTemporalData(), SceneUI({"LG Dwarves":5.0, "Volumetric AGORA":1.0, "M33":1.0, "Andromeda":1.0})), 10.0, "Milky Way", "mw"),
    # Illustris
    Transition(Scene(SceneSpatialData(Universe, 0.2e+25),
          SceneTemporalData(), SceneUI({"IllustrisTNG":1.0})), 10.0, "IllustrisTNG", "illustris", 100.0, 100.0),
    # SDSS distant
    Transition(Scene(SceneSpatialData(Universe, 1.0e+26),
          SceneTemporalData(), SceneUI({"SDSS":1.0})), 10.0, "SDSS", "sdss", 10000, 10000),
    #CMB
    Transition(Scene(SceneSpatialData(Universe, 1.0e+27),
          SceneTemporalData(), SceneUI({"SDSS":1.0, "CMB":1.0})), 10.0, "CMB", "", 10.0, 0.0),
]

def initScene():
    Animator.removeAllTransitions()
    for t in transitions:
        Animator.appendTransition(t)

    foo=0
    for t in transitions:
        foo += t.getDuration()
    print(foo)
    Animator.debug = False
    ToneMappingModel.contrast = 1.0
    Animator.setFirstScene()
    #Animator.restart()

    # display tweaks
    Universe.debrisSize=2
    Universe.setCosmoSimForcedQuality("Volumetric AGORA", 3)
