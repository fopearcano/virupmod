from PythonQt.QtGui import QKeyEvent
from PythonQt.QtCore import QElapsedTimer
from PythonQt.QtCore import Qt
from PythonQt.QtCore import QDateTime
from PythonQt.QtCore import QDate
from PythonQt.QtCore import QTime
from PythonQt.libplanet import Vector3
from PythonQt.VIRUP import UniverseElement
from math import exp
from math import log
from math import asin
from math import isnan
from math import atan2


departuretime = QDateTime(QDate(1977, 8, 20), QTime(20, 00, 00))
jupitertime = QDateTime(QDate(1979, 7, 5), QTime(22, 00, 00))
jupitertime2 = QDateTime(QDate(1979, 7, 13), QTime(22, 00, 00))
saturntime = QDateTime(QDate(1981, 8, 24), QTime(3, 24, 00))
saturntime2 = QDateTime(QDate(1981, 8, 28), QTime(3, 24, 00))
uranustime = QDateTime(QDate(1986, 1, 22), QTime(18, 00, 00))
uranustime2 = QDateTime(QDate(1986, 1, 26), QTime(18, 00, 00))
neptunetime = QDateTime(QDateTime(QDate(1989, 8, 23), QTime(4, 00, 00)))
neptunetime2 = QDateTime(QDateTime(QDate(1989, 8, 27), QTime(4, 00, 00)))
now = QDateTime.currentDateTime()

timings=[departuretime, jupitertime, jupitertime2, saturntime, saturntime2, uranustime, uranustime2, neptunetime, neptunetime2, now]
targets=["Jupiter", "Jupiter", "Saturn", "Saturn", "Uranus", "Uranus", "Neptune", "Neptune", "Sun", "Sun"]

personheight=1.5

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

def initScene():
    global current
    global init
    init=False
    VIRUP.labelsEnabled=True
    VIRUP.orbitsEnabled=True
    ToneMappingModel.exposure=8

def updateScene():
    global current
    global init
    if not VIRUP.isServer:
        return

    if not init:
        init = True
        current = 0
        VIRUP.simulationTime =departuretime
        VIRUP.timeCoeff = timings[current].secsTo(timings[current+1]) / 10.0
    VIRUP.scale = 0.1
    VIRUP.planetarySystemName = 'Solar System'

    VIRUP.planetTarget = 'Voyager 2'
    VIRUP.planetPosition = Vector3() + getPlanetShift()
    VIRUP.cosmoPosition = Vector3() + getCosmoShift()

    if current == len(timings)-1:
        VIRUP.timeCoeff=1.0
        return

    t = 1.0 - (float(VIRUP.simulationTime.secsTo(timings[current+1])) / timings[current].secsTo(timings[current+1]))

    # put t closer to 0
    t *= t

    pos=VIRUP.getCelestialBodyPosition(targets[current], "Voyager 2", VIRUP.simulationTime).getUnitForm()
    if current <= len(timings)-2:
        nextpos=VIRUP.getCelestialBodyPosition(targets[current+1], "Voyager 2", VIRUP.simulationTime).getUnitForm()
        pos = ((1-t)*pos + t*nextpos).getUnitForm()


    if not isnan(pos[0]) and not isnan(pos[1]) and not isnan(pos[2]):
        VIRUP.camYaw = atan2(-pos[1], -pos[0])
        VIRUP.camPitch=asin(pos[2])
        VIRUP.planetPosition = -15.0*pos - 7.5*Vector3.crossProduct(pos, Vector3(0.0, 0.0, 1.0))


    if VIRUP.simulationTime.secsTo(timings[current+1]) < 0.0:
        current += 1
        if current == len(timings)-1:
            VIRUP.timeCoeff=1.0
        else:
            VIRUP.timeCoeff = timings[current].secsTo(timings[current+1]) / 10.0

