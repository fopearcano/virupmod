from PythonQt.QtCore import Qt, QDateTime, QDate, QTime, QTimeZone
from PythonQt.QtMultimedia import QSound
from PythonQt.libplanet import Vector3
from PythonQt.virup import Transition, Scene, SceneSpatialData, SceneTemporalData, SceneUI, SceneCameraData, SceneToneMappingData

# CUSTOM
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

def earth(t, t_harsh):
    l=["Earth", "Moon", "Sun"]
    Universe.setLabelsOrbitsOnly(l)

def phobos(t, t_harsh):
    if t < 0.5:
        showOrbitsWhileTraveling(t, t_harsh)
    else:
        showOrbitsWhileTraveling(0.5, 0.5)
    l=["Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Phobos", "Deimos"]
    Universe.setLabelsOrbitsOnly(l)

def jupiter(t, t_harsh):
    if t < 0.5:
        showOrbitsWhileTraveling(t, t_harsh)
    else:
        showOrbitsWhileTraveling(0.5, 0.5)
    l=["Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Io", "Europa", "Ganymede", "Callisto"]
    Universe.setLabelsOrbitsOnly(l)

def saturn(t, t_harsh):
    if t < 0.5:
        showOrbitsWhileTraveling(t, t_harsh)
    else:
        showOrbitsWhileTraveling(0.5, 0.5)
    l=["Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Mimas", "Enceladus", "Dione", "Tethys", "Rhea", "Hyperion", "Titan", "Iapetus"]
    Universe.setLabelsOrbitsOnly(l)
# END CUSTOM

#2021-06-26T01:52:07Z
#2021-06-25T22:00:07Z
startdt = QDateTime(QDate(2016, 3, 24), QTime(10, 45, 00), QTimeZone(0))

transitions = [
# Intro
    # Earth
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'ISS', 120),
        SceneTemporalData(1.0, startdt), SceneUI({"Hipparcos":0.1})), 10.0, "ISS"),
    # Earth
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Earth', 30000000),
        SceneTemporalData(1.0), SceneUI({"Hipparcos":0.1, "PlanetsLabels":1.0})), 10.0, "Earth", "earth"),
    # Debris
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Earth', 40000000),
          SceneTemporalData(1000.0), SceneUI({"Gaia":1.0, "Hipparcos":0.1, "Debris":1.0})), 10.0, "Debris", "earth"),
    # Phobos
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Phobos', 30000),
       SceneTemporalData(500.0), SceneUI({"Hipparcos":0.1*0.3/0.5, "Orbits":1.0, "PlanetsLabels":1.0}),
       SceneCameraData(), SceneToneMappingData(0.5)), 10.0, 'Phobos', 'phobos'),
    # Jupiter
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Jupiter', 300000000),
       SceneTemporalData(1000.0), SceneUI({"Hipparcos":0.1*0.3/1.0, "Orbits":1.0, "PlanetsLabels":1.0}),
       SceneCameraData(), SceneToneMappingData(1.0)), 10.0, 'Jupiter', 'jupiter'),
    # Saturn
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Saturn', 300000000),
       SceneTemporalData(1000.0), SceneUI({"Hipparcos":0.01, "Orbits":1.0, "PlanetsLabels":1.0}),
       SceneCameraData(), SceneToneMappingData(3.0)), 10.0, 'Saturn', 'saturn'),
    #Solar System
    Transition(Scene(SceneSpatialData(Universe, 'Solar System', 'Sun', 1.65181e+12),
      SceneTemporalData(10000000.0), SceneUI({"Hipparcos":0.1, "Constellations":0.0, "Orbits":1.0, "PlanetsLabels":1.0, "Asteroids":1.0, "Constellations":1.0})), 10.0, "Solar System"),
    # Milky Way
    Transition(Scene(SceneSpatialData(Universe, 6.171e+20),#, Vector3(-0.43, -8.24, -0.81)),
          SceneTemporalData(), SceneUI({"LG Dwarves":5.0, "Volumetric AGORA":1.0, "M33":1.0, "Andromeda":1.0})), 10.0, "Milky Way"),
    # Illustris
    Transition(Scene(SceneSpatialData(Universe, 0.2e+25),
          SceneTemporalData(), SceneUI({"IllustrisTNG":1.0}),
          SceneCameraData(), SceneToneMappingData(0.3, 1.3)), 10.0, "IllustrisTNG"),
    # SDSS distant
    Transition(Scene(SceneSpatialData(Universe, 1.0e+26),
          SceneTemporalData(), SceneUI({"SDSS":1.0,})), 10.0, "SDSS"),
    #CMB
    Transition(Scene(SceneSpatialData(Universe, 1.0e+27),
          SceneTemporalData(), SceneUI({"SDSS":1.0, "CMB":1.0})), 10.0, "CMB"),
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
    Animator.setFirstScene()
    #Animator.restart()

    # display tweaks
    Universe.debrisSize=2
    Universe.setCosmoSimForcedQuality("Volumetric AGORA", 3)
