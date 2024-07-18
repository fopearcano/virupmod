from PythonQt.QtCore import Qt, QDateTime, QDate, QTime, QTimeZone
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
solareclipsedt = QDateTime(QDate(2021, 6, 26), QTime(1, 40, 30), QTimeZone(0))
isspos=Vector3(50, -50, 30)

transitions = [
# Intro
    # Illustris
    Transition(Scene(SceneSpatialData(Universe, 0.2e+25, Vector3(-0.43, -8.24, -0.81)),
        SceneTemporalData(), SceneUI({"IllustrisTNG 0":1.0, "IllustrisTNG 1":1.0, "IllustrisTNG 2":1.0, "IllustrisTNG 3":1.0, "IllustrisTNG 4":1.0, "IllustrisTNG 5":1.0,})), 1.0, "IllustrisTNG"),
]

def initScene():
    Animator.removeAllTransitions()
    for t in transitions:
        Animator.appendTransition(t)

    Animator.setFirstScene()
