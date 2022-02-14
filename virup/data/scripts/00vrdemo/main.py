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

def begin(t, t_harsh):
    Animator.fadeFactor = fade_in_factor(t_harsh)

def end(t, t_harsh):
    Animator.fadeFactor = fade_out_factor(t_harsh)

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
          SceneTemporalData(1.0, solareclipsedt), SceneUI({"Hipparcos":1.0})), 1.0, "begin", "begin"),
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

def initScene():
    Animator.removeAllTransitions()
    for t in transitions:
        Animator.appendTransition(t)

    Animator.setFirstScene()
