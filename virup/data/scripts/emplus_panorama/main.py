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
    SceneSpatialData.setForceDirectInterpolation(True)

def forceDirectTransitionAndAngle(t, t_harsh):
    forceDirectTransition(t, t_harsh)
    forceHorizontalPiOver2(t, t_harsh)

def sdss(t, t_harsh):
    if t < 0.1:
        Universe.setVisibility("IllustrisTNG", 1.0 - (10*t))
    else:
        Universe.setVisibility("IllustrisTNG", 0.0)

def debris(t, t_harsh):
    forceDirectTransitionAndAngle(t, t_harsh)
    Universe.setVisibility("Debris", 1.0)

def saturn1(t, t_harsh):
    forceHorizontalPiOver2(t, t_harsh)
    showOrbitsWhileTraveling(t, t_harsh)
    l=["Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"]
    Universe.setLabelsOrbitsOnly(l)
    ToneMappingModel.exposure=3.0
    Universe.setVisibility("Hipparcos", 0.01)
    Universe.setVisibility("Hipparcos", 0.01)

def saturn(t, t_harsh):
    forceHorizontalPiOver2(t, t_harsh)
    l=["Mimas", "Enceladus", "Dione", "Tethys", "Rhea", "Hyperion", "Titan", "Iapetus"]
    Universe.setLabelsOrbitsOnly(l)
    ToneMappingModel.exposure=3.0
    Universe.setVisibility("Hipparcos", 0.01)
    Universe.setVisibility("Hipparcos", 0.01)

def mw(t, t_harsh):
    Animator.shiftHorizontalAngle = (1-t) * 3.1415/2
    if t < 0.99:
        Universe.setVisibility("Exoplanets", (0.99-t)**1.2)
    else:
        Universe.setVisibility("Exoplanets", 0.0)
    print(t)

def wait(t, t_harsh):
    Animator.shiftHorizontalAngle = 20*t**0.7

def asteroids(t, t_harsh):
    forceDirectTransitionAndAngle(t, t_harsh)
    Universe.setVisibility("Asteroids", t**0.1)

# END CUSTOM

#2021-06-26T01:52:07Z
#2021-06-25T22:00:07Z
startdt = QDateTime(QDate(2021, 5, 31), QTime(10, 45, 00), QTimeZone(0))
isspos = Vector3(-48, 37, 20) # -50 0 30


transitions = [
# Intro
    # ISS
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 1, isspos),
          SceneTemporalData(1.0, startdt), SceneUI({"Hipparcos":0.1})), 1.0, "", "begin"),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 1, isspos),
          SceneTemporalData(1.0), SceneUI({"Hipparcos":0.1})), 10.0, "International Space Station", "forceHorizontalPiOver2"),

    # Earth
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Earth', 15000000),
          SceneTemporalData(10.0), SceneUI({"Gaia":1.0, "Hipparcos":0.1})), 50.0, "Earth", "forceDirectTransitionAndAngle", 0.0, 0.09),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Earth', 40000000),
          SceneTemporalData(1000.0), SceneUI({"Gaia":1.0, "Hipparcos":0.1, "Debris":1.0})), 50.0, "Debris", "debris", 10000.0, 10000.0),
    # Solar System dynamics Constellations
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Earth', 5.65181e+10),
       SceneTemporalData(1000.0), SceneUI({"Hipparcos":0.1})), 10.0, "Solar System", "forceDirectTransitionAndAngle"),
    # Saturn
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Saturn', 300000000),
       SceneTemporalData(1000.0), SceneUI({"Hipparcos":0.1})), 20.0, 'Saturn', 'saturn1'),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Saturn', 300000000),
       SceneTemporalData(1000.0), SceneUI({"Hipparcos":0.1, "Constellations":0.0, "Orbits":1.0, "PlanetsLabels":1.0})), 40.0, "", "saturn"),
    #Solar System
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Saturn', 1.65181e+12),
      SceneTemporalData(10000000.0), SceneUI({"Hipparcos":0.1, "Constellations":0.0, "Orbits":1.0, "PlanetsLabels":1.0, "Asteroids":0.0, "Constellations":1.0})), 30.0, "Solar System", "forceDirectTransitionAndAngle"),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Saturn', 5.65181e+12),
      SceneTemporalData(10000000.0), SceneUI({"Hipparcos":0.1, "Constellations":0.0, "Orbits":1.0, "PlanetsLabels":1.0, "Asteroids":1.0, "Constellations":1.0})), 30.0, "Solar System", "asteroids"),
    # Kepler-11
    Transition(Scene(SceneSpatialData(Universe, 'Kepler-11', 'Kepler-11', 1e11, Vector3(0.0, 0.0, 0.0)),
          SceneTemporalData(20000.0), SceneUI({"Hipparcos":0.1, "Constellations":0.0, "Orbits":1.0, "PlanetsLabels":1.0, "Constellations":1.0})), 20.0, "", "forceHorizontalPiOver2"),
    Transition(Scene(SceneSpatialData(Universe, 'Kepler-11', 'Kepler-11', 1e11, Vector3(0.0, 0.0, 0.0)),
          SceneTemporalData(20000.0), SceneUI({"Hipparcos":0.1, "Constellations":0.0, "Orbits":1.0, "PlanetsLabels":1.0})), 30.0, "Kepler-11", "forceHorizontalPiOver2"),
    # Milky Way
    Transition(Scene(SceneSpatialData(Universe, 6.171e+20),#, Vector3(-0.43, -8.24, -0.81)),
          SceneTemporalData(), SceneUI({"LG Dwarves":5.0, "Volumetric AGORA":1.0, "Exoplanets":1.0})), 20.0, "Milky Way", "forceDirectTransitionAndAngle"),
    Transition(Scene(SceneSpatialData(Universe, 6.171e+20),#, Vector3(-0.43, -8.24, -0.81)),
          SceneTemporalData(), SceneUI({"LG Dwarves":5.0, "Volumetric AGORA":1.0})), 40.0, "Milky Way", "mw"),
    # Local Group
    Transition(Scene(SceneSpatialData(Universe, 3.04e+22),
          SceneTemporalData(), SceneUI({"IllustrisTNG":0.1, "Volumetric AGORA":1.0, "Andromeda":2.0, "M33":2.0, "LG Dwarves":5.0})), 50.0, "Local Group", "", 0.0, 0.2),
    # Illustris
    Transition(Scene(SceneSpatialData(Universe, 0.9e+24),
          SceneTemporalData(), SceneUI({"IllustrisTNG":1.0})), 125.0, "IllustrisTNG", "", 100.0, 100.0),
    # SDSS distant
    Transition(Scene(SceneSpatialData(Universe, 1.0e+24),
          SceneTemporalData(), SceneUI({"SDSS":1.0})), 15.0, "SDSS", "sdss", 10000, 10000),
    Transition(Scene(SceneSpatialData(Universe, 1.0e+24, Vector3(1.2e7, 0, 0)), # + Vector3(-0.43, -8.24, -0.81)
          SceneTemporalData(), SceneUI({"SDSS":1.0, "CMB":0.0})), 130.0, "", "forceDirectTransition", 0.003, 0.1),
    Transition(Scene(SceneSpatialData(Universe, 1.0e+27),
          SceneTemporalData(), SceneUI({"SDSS":1.0, "CMB":1.0})), 60.0, "CMB", "end", 10.0, 0.0),
    Transition(Scene(SceneSpatialData(Universe, 1.0e+27),
          SceneTemporalData(), SceneUI({"SDSS":1.0, "CMB":1.0})), 197.0, "", "wait", 0.0, 10000.0),

    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Earth', 15000000),
          SceneTemporalData(1.0), SceneUI({"Gaia":1.0, "Hipparcos":0.1})), 1.0, "", ""),
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Earth', 15000000),
          SceneTemporalData(1.0), SceneUI({"Gaia":1.0, "Hipparcos":0.1})), 1.0, "", "")
]

def initScene():
    Animator.removeAllTransitions()
    for t in transitions:
        Animator.appendTransition(t)

    foo=0
    for t in transitions:
        foo += t.getDuration()
    print(foo)

    #Animator.setFirstScene()
    #Animator.restart()
