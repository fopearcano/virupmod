from PythonQt.QtGui import QKeyEvent
from PythonQt.QtCore import QElapsedTimer
from PythonQt.QtCore import Qt
from PythonQt.QtCore import QDateTime
from PythonQt.QtCore import QDate
from PythonQt.QtCore import QTime
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
    ms0 = dt0.toMSecsSinceEpoch()#currentscene.temporalData.simulationTime.toMSecsSinceEpoch()
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
        bn = Universe.getClosestCommonAncestorName(s0.bodyName, s1.bodyName)
        planetpos = Universe.interpolateCoordinates(s0.bodyName, s1.bodyName, t)

        start=Universe.getCelestialBodyPosition(s0.bodyName, bn, simTime0)
        end=Universe.getCelestialBodyPosition(s1.bodyName, bn, simTime1)

        dist = (end-start).length()
        ### FOR MOVIE
        dist /= 3.0
        ### END FOR MOVIE
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

class Timer:
    def __init__(self):
        self.time = 0
    def restart(self):
        self.time = 0
    def elapsed(self):
        return self.time
    def advance(self, dt):
        self.time += dt

def updateTimer(frameTiming):
    global timer
    timer.advance(1000.0*frameTiming)

####### CUSTOM FUNCTIONS

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
    Universe.scale=interpolateLog(scale0, scale1, t_harsh)

def black(t, t_harsh):
    global fade_factor
    fade_factor = 0.0

def intro_phobos(t, t_harsh):
    intro(t, t_harsh)
    pos=Universe.getCelestialBodyPosition("Mars", "Phobos", Universe.simulationTime).getUnitForm()
    if not isnan(pos[0]) and not isnan(pos[1]) and not isnan(pos[2]):
        Universe.camYaw = atan2(-pos[1], -pos[0])
        Universe.camPitch=asin(pos[2])
        Universe.planetPosition = -60000.0*pos - 30000*Vector3.crossProduct(pos, Vector3(0.0, 0.0, 1.0)) - getPlanetShift()

def intro_mw(t, t_harsh):
    global shiftangle
    intro(t, t_harsh)
    Universe.setAnimationTime(interpolateLinear(0.6, 0.8, t_harsh))
    shiftangle = interpolateLinear(0.0, pi/3.0, t_harsh)

def intro_sdss(t, t_harsh):
    global shiftangle
    intro(t, t_harsh)
    shiftangle = interpolateLinear(0.0, 0.5, t_harsh)

def lookatiss(t, t_harsh):
    direc=(Universe.getCameraCurrentRelPosToBody("ISS")+Vector3(-80, 50, 0) - getPlanetShift()).getUnitForm()
    Universe.camYaw = atan2(-direc[1], -direc[0])
    Universe.camPitch=asin(direc[2])

def lookatiss_surface(t, t_harsh):
    global fade_factor
    lookatiss(t, t_harsh)
    fade_factor = fade_in_factor(t_harsh)
    Universe.setLabelsOrbitsOnly(["ISS"])
    Universe.setVisibility("PlanetsLabels", fade_factor)

def isszoomout0(t, t_harsh):
    scenebeg=scenes[getIdFromName("ISS3")]
    scenemid=scenes[getIdFromName("ISS3")+1]
    sceneend=scenes[getIdFromName("ISS3")+2]
    """
    coeff = scenemid.transitiontimeto / (sceneend.transitiontimeto + scenemid.transitiontimeto)
    t_new = smoothstep(t_harsh * coeff) / smoothstep(coeff)
    """
    t_new = smoothstep(t_harsh, 0.0, 10.0)
    Universe.scale = interpolateLog(scenebeg.spatialData.scale, scenemid.spatialData.scale, t_new)
    Universe.planetPosition = spatialData.planetPos

def isszoomout1(t, t_harsh):
    scenebeg=scenes[getIdFromName("ISS3")]
    scenemid=scenes[getIdFromName("ISS3")+1]
    sceneend=scenes[getIdFromName("ISS3")+2]
    """
    coeff = sceneend.transitiontimeto / (sceneend.transitiontimeto + scenemid.transitiontimeto)
    tmid = 1.0 - coeff
    t_new = smoothstep(t_harsh * coeff + tmid) - smoothstep(tmid)
    """
    t_new = smoothstep(t_harsh, 10.0, 0.0)
    Universe.scale = interpolateLog(scenebeg.spatialData.scale, scenemid.spatialData.scale, t_new)
    Universe.planetPosition = sceneend.spatialData.planetPos


planetposbak = Vector3()
departure1 = Vector3()
def triptoiss(t, t_harsh):
    global planetposbak
    global departure1
    global shiftangle

    shiftangle = pi
    t_new = t**0.1
    Universe.scale = interpolateLog(1.0, 100.0, t_new)
    Universe.setLabelsOrbitsOnly(["ISS"])

    departure0=Vector3(-1.8695e+06, -1.80667e+06, 5.81506e+06)
    arrival0=(Universe.getCelestialBodyPosition("ISS", "Earth", Universe.simulationTime)+Vector3(-80, 50, 0))
    target0="Earth"

    if t_new < 0.8:
        Universe.planetTarget=target0
        Universe.planetPosition= t_new*arrival0+(1-t_new)*departure0
        planetposbak = Universe.planetPosition
        departure1 = Vector3()
        lookatiss(t, t_harsh)
        return

    departure1=departure0+(Universe.getCelestialBodyPosition("Earth", "ISS", Universe.simulationTime)-Vector3(-80, 50, 0))

    if departure1[0] == 0 and departure1[1] == 0 and departure1[2] == 0:
        departure1=planetposbak-Universe.getCelestialBodyPosition("ISS", "Earth", Universe.simulationTime)
    arrival1=Vector3(-229.81, 50, 7.5) #- Vector3(70, 50, 7.52)
    target1="ISS"

    Universe.planetTarget=target1
    Universe.planetPosition= t_new*arrival1+(1-t_new)*departure1
    lookatiss(t, t_harsh)

def turnaroundiss(t, t_harsh):
    global shiftangle
    global shiftvertangle

    shiftangle=interpolateLinear(pi, 2*pi, t)
    if t_harsh < 0.5:
        shiftvertangle=interpolateLinear(0.05, pi/4, smoothstep(t_harsh*2.0))
    else:
        shiftvertangle=interpolateLinear(pi/4, 0.05, smoothstep((t_harsh-0.5)*2.0))

    if t_harsh < 0.01:
        print(Universe.planetPosition + getPlanetShift())


def issout(t, t_harsh):
    departure=Vector3(-80, 50, 0)
    arrival=Universe.getCelestialBodyPosition("Earth", "ISS", Universe.simulationTime)
    t_new = t**10
    Universe.planetPosition = t_new*arrival+(1-t_new)*departure

def earthtomoon(t, t_harsh):
    global shiftangle
    id0=getIdFromName("debris2")
    id1=getIdFromName("moon")
    if t < 0.5:
        t_new = smoothstep(t_harsh*2.0)
        Universe.scale = interpolateLog(scenes[id0].spatialData.scale, 1.0/180000000, t_new)
    else:
        t_new = smoothstep((t_harsh-0.5)*2.0)**5
        Universe.scale = interpolateLog(1.0/180000000, scenes[id1].spatialData.scale, t_new)
    departure=Vector3(0, 0, 0)
    arrival=Universe.getCelestialBodyPosition("Moon", "Earth", Universe.simulationTime)
    t_new = t_harsh*1.2
    t_new = clamp(t_new, 0.0, 1.0)
    t_new = smoothstep(t_new)
    Universe.planetPosition = interpolateLinear(departure, arrival, t_new**0.9)
    shiftangle=interpolateLinear(0.0, 1.13*pi/2, t**6.0)
    pos=-1*getPlanetShift().getUnitForm()
    nextpos=(-1*Universe.planetPosition-1*getPlanetShift()).getUnitForm()
    pos=interpolateLinear(pos, nextpos, t**2)
    if not isnan(pos[0]) and not isnan(pos[1]) and not isnan(pos[2]):
        Universe.camYaw = atan2(-pos[1], -pos[0])
        Universe.camPitch=asin(pos[2])

def moon(t, t_harsh):
    global shiftangle
    shiftangle=interpolateLinear(1.13*pi/2, 0.0, t)
    pos=(-1*getPlanetShift()+Universe.getCelestialBodyPosition("Earth", "Moon", Universe.simulationTime)).getUnitForm()
    nextpos=-1*getPlanetShift().getUnitForm()
    pos=interpolateLinear(pos, nextpos, t**0.5)
    if not isnan(pos[0]) and not isnan(pos[1]) and not isnan(pos[2]):
        Universe.camYaw = atan2(-pos[1], -pos[0])
        Universe.camPitch=asin(pos[2])

def showOrbitsWhileTraveling(t, t_harsh):
    if t_harsh > 0.3 and t_harsh < 0.7:
        Universe.setVisibility("PlanetsLabels", 1.0)
        Universe.setVisibility("Orbits", 1.0)
    elif t_harsh <= 0.3:
        Universe.setVisibility("PlanetsLabels", (t_harsh*3)**5)
        Universe.setVisibility("Orbits", (t_harsh*3)**5)
    elif t_harsh >= 0.7:
        Universe.setVisibility("PlanetsLabels", (1.0 - (t_harsh-0.7)*3)**5)
        Universe.setVisibility("Orbits", (1.0 - (t_harsh-0.7)*3)**5)

moondt=0.0
moonscale=0.0
def moontophobos(t, t_harsh):
    global moondt
    global moonscale
    if moondt==0.0:
        moondt = Universe.simulationTime
        moonscale=Universe.scale
    Universe.simulationTime = interpolateDateTime(moondt, phobosdt, t)
    showOrbitsWhileTraveling(t, 0.6*(t_harsh-0.5) + 0.5)
    Universe.setLabelsOrbitsOnly(["Moon", "Earth", "Mars", "Mercury", "Venus", "Jupiter", "Phobos", "Deimos", "Saturn", "Uranus", "Neptune", "Sun"])
    if t_harsh < 1.0/2.0:
        t_new = smoothstep(2*t_harsh)
        Universe.scale = interpolateLog(moonscale, 4e-12, t_new)
    elif t_harsh > 1.0/2.0:
        t_new = 1.0 - smoothstep(2*(t_harsh-1.0/2.0))
        Universe.scale = interpolateLog(1.0/3000000, 4e-12, t_new)
    else:
        Universe.scale = 4e-12

    if t_harsh < 1.0/3.0 or t_harsh > 2.0/3.0:
        if t_harsh < 1.0 / 3.0:
            Universe.planetTarget = "Moon"
        else:
            Universe.planetTarget = "Phobos"
        Universe.planetPosition = Vector3()
    else:
        t_new = smoothstep((t_harsh-1.0/3.0)*3.0)
        Universe.planetTarget = "Sun"
        moonpos=Universe.getCelestialBodyPosition("Moon", "Sun", Universe.simulationTime)
        phobospos=Universe.getCelestialBodyPosition("Phobos", "Sun", Universe.simulationTime)
        Universe.planetPosition=interpolateLinear(moonpos, phobospos, t_new)

def phobosorbit(t, t_harsh):
    pos=Universe.getCelestialBodyPosition("Mars", "Phobos", Universe.simulationTime).getUnitForm()
    nextpos=-1*getPlanetShift().getUnitForm()
    if t < 0.5:
        t_new = t_harsh*2.0
    else:
        t_new = 2.0 - 2.0 * t_harsh
    t_new = smoothstep(t_new)
    pos = t_new*pos + (1.0-t_new)*nextpos
    if not isnan(pos[0]) and not isnan(pos[1]) and not isnan(pos[2]):
        Universe.camYaw = atan2(-pos[1], -pos[0])
        Universe.camPitch=asin(pos[2])

def phobosend(t, t_harsh):
    global id
    fade_out(t, t_harsh)
    scale0=scenes[id-1].spatialData.scale
    scale1=scenes[id].spatialData.scale
    if t_harsh < 0.5:
        Universe.scale = interpolateLog(scale0, scale1, t)
    else:
        Universe.scale = interpolateLog(scale0, scale1, t_harsh)

def phobosToVoyager(t, t_harsh):
    black(t, t_harsh)
    Universe.planetTarget = "Voyager 2"
    Universe.planetPosition = Vector3(-11.593, -16.4098, -0.12561)
    Universe.camYaw = -0.903532683849
    Universe.camPitch = -0.0416171476245

def voyagerCamera(t, t_harsh):
    global id
    global jupiterendid
    global fade_factor

    ToneMappingModel.exposure = 0.3 * (Universe.getCameraCurrentRelPosToBody("Sun").length() / 4.0e+11)**2#7.779e+11)**2
    if id == jupiterendid and t_harsh < 0.333:
        fade_factor = fade_in_factor(t_harsh*3)

    targets=["Jupiter", "Saturn", "Saturn", "Uranus", "Uranus", "Neptune", "Neptune", "End"]
    t_voyager = t**3
    current = id - jupiterendid
    pos=Universe.getCameraCurrentRelPosToBody(targets[current]).getUnitForm()
    last=False
    if current <= len(targets)-3:
        nextpos=Universe.getCameraCurrentRelPosToBody(targets[current+1]).getUnitForm()
    else:
        last=True
        nextpos=-1*getPlanetShift().getUnitForm()
    if pos != nextpos:
        pos = ((1-t_voyager)*pos + t_voyager*nextpos).getUnitForm()
    if not isnan(pos[0]) and not isnan(pos[1]) and not isnan(pos[2]):
        newcamyaw = atan2(-pos[1], -pos[0])
        newcampitch = asin(pos[2])

        Universe.camYaw = newcamyaw
        Universe.camPitch= newcampitch
        Universe.planetPosition = -15.0*pos - 7.5*Vector3.crossProduct(pos, Vector3(0.0, 0.0, 1.0)) - getPlanetShift()
        if last:
            Universe.planetPosition *= (1-t)

def voyagerToSS1(t, t_harsh):
    if t < 0.2:
        ToneMappingModel.exposure = 0.3 * (Universe.getCameraCurrentRelPosToBody("Sun").length() / 4.0e+11)**2#7.779e+11)**2
    t_new=t_harsh-0.55
    t_new *= 1/(1.0-0.55)
    t_new=clamp(t_new, 0.0, 1.0)
    Universe.setVisibility("Orbits", smoothstep(t_new))

def voyagerToSS2(t, t_harsh):
    global shiftvertangle
    shiftvertangle = interpolateLinear(0.05, 0.2, t)

    Universe.planetTarget = "Sun"
    departure=Universe.getCelestialBodyPosition("Voyager 2", "Sun", Universe.simulationTime)
    arrival=Vector3(0.0, 0.0, 0.0)
    Universe.planetPosition = interpolateLinear(departure, arrival, t)

    obts=["Uranus", "Neptune", "Pluto", "Haumea", "Makemake", "Eris", "FarFarOut", "New Horizons", "Voyager 1", "Voyager 2", "Pioneer 10", "Pioneer 11", "Sun"]
    Universe.setLabelsOrbitsOnly(obts)

def outerSS(t, t_harsh):
    global shiftvertangle
    shiftvertangle = 0.2
    obts=["Uranus", "Neptune", "Pluto", "Haumea", "Makemake", "Eris", "FarFarOut", "New Horizons", "Voyager 1", "Voyager 2", "Pioneer 10", "Pioneer 11", "Sun"]
    Universe.setLabelsOrbitsOnly(obts)

def alpCen(t, t_harsh):
    global shiftvertangle
    shiftvertangle = 0.2
    if t_harsh > 0.3:
        obts=["Proxima Centauri", "Proxima Centauri b", "Proxima Centauri c", "Proxima Centauri d"]
        if t_harsh < 0.35:
            alpCendt = QDateTime(QDate(2031, 11, 11), QTime(0, 0, 0))
            Universe.simulationTime = alpCendt
    else:
        obts=["Uranus", "Neptune", "Pluto", "Haumea", "Makemake", "Eris", "FarFarOut", "New Horizons", "Voyager 1", "Voyager 2", "Pioneer 10", "Pioneer 11", "Sun"]
    Universe.setLabelsOrbitsOnly(obts)

def alpCen2(t, t_harsh):
    global shiftangle
    global shiftvertangle
    shiftangle=interpolateLinear(0.0, pi, t)
    shiftvertangle = 0.2
    obts=["Proxima Centauri", "Proxima Centauri b", "Proxima Centauri c", "Proxima Centauri d"]
    Universe.setLabelsOrbitsOnly(obts)

def peg51(t, t_harsh):
    global shiftangle
    global shiftvertangle
    shiftangle=pi
    shiftvertangle = interpolateLinear(0.2, 0.05, t)
    if t_harsh > 0.5:
        Universe.setVisibility("Orbits", 0.0)
        if t_harsh < 0.55:
            peg51dt = QDateTime(QDate(2031, 12, 20), QTime(9, 0, 0))
            Universe.simulationTime = peg51dt
    else:
        obts=["Proxima Centauri", "Proxima Centauri b", "Proxima Centauri c", "Proxima Centauri d"]
        Universe.setLabelsOrbitsOnly(obts)

def peg512(t, t_harsh):
    global shiftangle
    global shiftvertangle
    shiftangle=interpolateLinear(pi, 2.0*pi, t)

currCosmoPos=Vector3(-1, 0, 0)
def radio(t, t_harsh):
    global currCosmoPos
    if currCosmoPos == Vector3(-1, 0, 0):
        currCosmoPos = Universe.cosmoPosition
    id51Peg2=getIdFromName("51peg2")
    pos0=scenes[id51Peg2].spatialData.cosmoPos
    pos1=Vector3()
    scale0=scenes[id51Peg2].spatialData.scale
    scale1=scenes[id51Peg2+1].spatialData.scale
    Universe.scale = interpolateLog(scale0, scale1, t**0.5)
    if t_harsh > 0.7:
        Universe.cosmoPosition = interpolateLinear(pos0, pos1, smoothstep((t_harsh-0.7)/0.3))
    elif currCosmoPos != Universe.cosmoPosition:
        Universe.cosmoPosition = currCosmoPos

def radio2(t, t_harsh):
    global shiftangle
    shiftangle=interpolateLinear(0, 2.0*pi, t)

def radioToMW(t, t_harsh):
    idRadio2=getIdFromName("radio2")
    scale0=scenes[idRadio2].spatialData.scale
    scale1=scenes[idRadio2+1].spatialData.scale
    Universe.scale = interpolateLog(scale0, scale1, t**0.4)

def makeOneTurnBef(t, t_harsh):
    global shiftangle
    shiftangle = 4.0*pi*smoothstep(t_harsh*0.5)

def makeOneTurnAft(t, t_harsh):
    global shiftangle
    shiftangle = 4.0*pi*smoothstep(t_harsh*0.5+0.5)

def makeOneTurn(t, t_harsh):
    global shiftangle
    shiftangle = 2.0*pi*t

def mwscalerot(t, t_harsh, scale=10.0):
    scale0=scenes[getIdFromName("mw")].spatialData.scale
    scale1=scale0*scale
    if t_harsh < 0.5:
        Universe.scale=interpolateLog(scale0, scale1, smoothstep(t_harsh*2.0))
    else:
        Universe.scale=interpolateLog(scale1, scale0, smoothstep(t_harsh*2.0 - 1.0))

def mwpart1(t, t_harsh):
    global shiftangle
    new_t = smoothstep(t_harsh, 0.0, 3.0)
    shiftangle = 2.0*pi*new_t

def mwpart2(t, t_harsh):
    global shiftangle
    shiftangle = 2.0*pi*t_harsh
    if t_harsh < 0.05:
        Universe.setVisibility("Exoplanets", 0.001*(1.0-smoothstep(t_harsh*20.0))**2)
    else:
        Universe.setVisibility("Exoplanets", 0.0)
    mwscalerot(t, t_harsh)

def mwdyn(t, t_harsh):
    makeOneTurnAft(t, t_harsh)
    mwscalerot(t, t_harsh, scale=2.0)

    t_new = 1.0 - clamp(t*30.0, 0.0, 1.0)
    t_new += clamp((t-0.95)/(1.0-0.95), 0.0, 1.0)
    Universe.setVisibility("Exoplanets", 0.0)
    Universe.setVisibility("LG Dwarves", t_new)
    Universe.setVisibility("Andromeda", t_new)
    Universe.setVisibility("M33", t_new)

    tbeg=10.0
    dur=45.0

    t_new = t_harsh
    t_new -= tbeg / 72.0
    t_new *= 72.0 / dur

    if t_new < 0.0:
        t_new = 0.0
    if t_new > 1.0:
        t_new = 1.0

    t_new = squeeze_t(t_new, 0.5)

    t_new = smoothstep(t_new)
    Universe.setAnimationTime(interpolateLinear(0.401, 1.0, t_new))

def lg1(t, t_harsh):
    global shiftangle
    global shiftvertangle
    shiftangle=interpolateLinear(0.0, 0.6, t)
    shiftvertangle=interpolateLinear(0.05, 0.43, t)
    t_new = t_harsh*10
    t_new = clamp(t_new, 0.0, 1.0)
    Universe.setVisibility("Labels", smoothstep(t_new))
    Universe.setVisibility("Labels2", smoothstep(t_new))
    if t_harsh > 0.4:
        t_new = t_harsh - 0.4
        t_new /= 0.5 - 0.4
        t_new = clamp(t_new, 0.0, 1.0)
        Universe.setVisibility("Labels", 1.0 - smoothstep(t_new))

def lg2(t, t_harsh):
    global shiftangle
    global shiftvertangle
    shiftangle=interpolateLinear(0.6, 0.0, t)
    shiftvertangle=interpolateLinear(0.43, 0.05, t)

def mwandr(t, t_harsh):
    t_new = t**0.25
    Universe.setAnimationTime(t_new)
    scale0=scenes[getIdFromName("mwandrbeg")].spatialData.scale
    scale1=scenes[getIdFromName("mwandr")].spatialData.scale
    Universe.scale = interpolateLog(scale0, scale1, t_new)

    pos0=scenes[getIdFromName("mwandrbeg")].spatialData.cosmoPos
    pos1=scenes[getIdFromName("mwandr")].spatialData.cosmoPos
    Universe.cosmoPosition = interpolateLinear(pos0, pos1, t_new**2)

valBAK=0.0
timeBAK=0.0

def debugSmooth(t_harsh, val, precision=0.02):
    global id
    global valBAK
    global timeBAK
    if t_harsh < precision or t_harsh > (1.0-precision):
        print(str(id) + " " + str((val - valBAK)/(timer.elapsed() - timeBAK)))
    valBAK = val
    timeBAK=timer.elapsed()

def sdss1(t, t_harsh):
    global shiftangle
    t_new = smoothstep(t_harsh, 0.0, 10.0)
    shiftangle = interpolateLinear(0.0, 0.2, t_new)
    debugSmooth(t_harsh, shiftangle)

def sdss2(t, t_harsh):
    global shiftangle
    t_new = smoothstep(t_harsh, 0.35, 1.0)
    shiftangle = interpolateLinear(0.2, pi, t_new)
    debugSmooth(t_harsh, shiftangle)

def cmb(t, t_harsh):
    global shiftangle
    t_new = smoothstep(t_harsh, 0.434, 0.0)
    shiftangle = interpolateLinear(pi, 2*pi, t_new)
    debugSmooth(t_harsh, shiftangle)

def cubeevolstart(t, t_harsh):
    global shiftangle
    shiftangle = interpolateLinear(0.0, pi/4.0, t)

def cubeevolstart2(t, t_harsh):
    global shiftangle
    shiftangle = interpolateLinear(pi/4.0, 0.0, t)

def cosmodyn(t, t_harsh):
    comov_dist = (Universe.cosmoPosition-getCosmoShift()).length()
    Universe.setAnimationTime(getAnimationTimeFromComovDist(comov_dist))

def lgback(t, t_harsh):
    scale0=scenes[getIdFromName("lgback")-1].spatialData.scale
    scale1=scenes[getIdFromName("lgback")].spatialData.scale
    t_new = smoothstep(t_harsh, 0.0, 10.0)
    Universe.scale = interpolateLog(scale0, scale1, t_new)
    debugSmooth(t_harsh, log(Universe.scale))

def mwback(t, t_harsh):
    scale0=scenes[getIdFromName("mwback")-1].spatialData.scale
    scale1=scenes[getIdFromName("mwback")].spatialData.scale
    t_new = smoothstep(t_harsh, 3.11, 10.0)
    Universe.scale = interpolateLog(scale0, scale1, t_new)
    debugSmooth(t_harsh, log(Universe.scale))

def ssback(t, t_harsh):
    global shiftvertangle
    shiftvertangle = interpolateLinear(0.05, 0.2, t)

    scale0=scenes[getIdFromName("ssback")-1].spatialData.scale
    scale1=scenes[getIdFromName("ssback")].spatialData.scale
    t_new = smoothstep(t_harsh, 0.325, 1.0)
    Universe.scale = interpolateLog(scale0, scale1, t_new)
    debugSmooth(t_harsh, log(Universe.scale))

def earthback(t, t_harsh):
    global shiftvertangle
    shiftvertangle = interpolateLinear(0.2, 0.05, t)
    showOrbitsWhileTraveling(t, t_harsh*0.5+0.5)

    scale0=scenes[getIdFromName("earthback")-1].spatialData.scale
    scale1=scenes[getIdFromName("earthback")].spatialData.scale
    t_new = smoothstep(t_harsh, 5.88, 0.0)
    Universe.scale = interpolateLog(scale0, scale1, t_new)
    debugSmooth(t_harsh, log(Universe.scale))

def end1(t, t_harsh):
    global shiftangle
    global fade_factor
    t_new = smoothstep(t_harsh, 0.0, 10.0)
    shiftangle = interpolateLinear(0.0, 1.5, t_new)

def end2(t, t_harsh):
    global shiftangle
    global fade_factor
    t_new = smoothstep(t_harsh, 0.4, 0.0)
    shiftangle = interpolateLinear(1.5, 3.23, t_new)
    if t > 0.8:
        fade_factor = fade_out_factor(5*(t-0.8))

####### END CUSTOM FUNCTIONS

isspassdt = QDateTime(QDate(2021, 2, 25), QTime(6, 29, 00))
phobosdt_intro = QDateTime(QDate(2021, 2, 25), QTime(11, 10, 00))
phobosdt = QDateTime(QDate(2021, 2, 25), QTime(9, 10, 00))
departuretime = QDateTime(QDate(1977, 8, 20), QTime(20, 00, 00))
jupitertime = QDateTime(QDate(1979, 7, 7), QTime(22, 00, 00))
#jupitertime2 = QDateTime(QDate(1979, 7, 13), QTime(22, 00, 00))
jupitertime2 = QDateTime(QDate(1979, 7, 10), QTime(3, 00, 00))
saturntime = QDateTime(QDate(1981, 8, 24), QTime(22, 24, 00))
#saturntime2 = QDateTime(QDate(1981, 8, 28), QTime(3, 24, 00))
saturntime2 = QDateTime(QDate(1981, 8, 26), QTime(8, 00, 00))
uranustime = QDateTime(QDate(1986, 1, 24), QTime(10, 00, 00))
#uranustime2 = QDateTime(QDate(1986, 1, 26), QTime(18, 00, 00))
uranustime2 = QDateTime(QDate(1986, 1, 24), QTime(19, 00, 00))
neptunetime = QDateTime(QDateTime(QDate(1989, 8, 24), QTime(16, 00, 00)))
#neptunetime2 = QDateTime(QDateTime(QDate(1989, 8, 27), QTime(4, 00, 00)))
neptunetime2 = QDateTime(QDateTime(QDate(1989, 8, 25), QTime(11, 30, 00)))
now = QDateTime.currentDateTime()
endtime = QDateTime(QDateTime(QDate(2021, 3, 24), QTime(11, 30, 00)))

#sdsslum, gaialum, illustrislum, agoralum, hyg, exoplanets, constellations, orbits, labels, cmb, andromeda):
scenes = [
    # Intro
    # Earth
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 40000000, 'Earth', 'Solar System'),
          TemporalData(4000.0, endtime), UI({"Gaia":1.0, "Hipparcos":1.0})),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 40000000, 'Earth', 'Solar System'),
          TemporalData(4000.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 6.0, "earthintro", intro),
    # Phobos
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 30000, 'Phobos', 'Solar System'),
          TemporalData(500.0, phobosdt_intro), UI({"Gaia":1.0, "Hipparcos":1.0}), 0.01, "foo0", black),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 30000, 'Phobos', 'Solar System'),
          TemporalData(500.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 6.0, "phobosintro", intro_phobos),
    # Milky Way
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 6.171e+20),
        TemporalData(), UI({"Dynamic AGORA":1.0}), 0.01, "foo1", black),
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 6.171e+20),
        TemporalData(), UI({"Dynamic AGORA":1.0}), 6.0, "mwintro", intro_mw),
    # Illustris
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 1.0e+24),
        TemporalData(), UI({"IllustrisTNG":1.0, "AnimationTime":1.0}), 0.01, "foo2", black),
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 2.0e+24),
        TemporalData(), UI({"IllustrisTNG":1.0, "AnimationTime":1.0}), 6.0, "illustris_intro", intro),
    # SDSS
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 2.0e+25),
        TemporalData(), UI({"SDSS":1.0}), 0.01, "foo3", black),
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 4.0e+25),
        TemporalData(), UI({"SDSS":1.0}), 6.0, "cmb_intro", intro_sdss),

    # Surface
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'Earth', 'Solar System', Vector3(-1.8695e+06, -1.80667e+06, 5.81506e+06)),
         TemporalData(1.0, isspassdt), UI({"Gaia":1.0, "Hipparcos":1.0, "PlanetsLabels":1.0}), 1.0, "begin", black),
    # Surface 2
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1, 'Earth', 'Solar System', Vector3(-1.8695e+06, -1.80667e+06, 5.81506e+06)),
          TemporalData(1.0), UI({"Gaia":1.0, "Hipparcos":1.0, "PlanetsLabels":1.0}), 10.0, "surface2", lookatiss_surface),
    # International Space Station Zoom-in
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 100, 'ISS', 'Solar System', Vector3(-80, 50, 0)),
          TemporalData(1.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 15.0, "ISS", triptoiss),
    # International Space Station
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 100, 'ISS', 'Solar System', Vector3(-80, 50, 0)),
        TemporalData(1.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 15.0, "ISS2", turnaroundiss),
    # International Space Station Zoom-out
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 40000000, 'ISS', 'Solar System', Vector3(-80, 50, 0)),
          TemporalData(1.0), UI({"Gaia":1.0, "Hipparcos":1.0, "Debris":1.0}), 25.0, "issout", issout),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 40000000, 'Earth', 'Solar System'),
          TemporalData(400.0), UI({"Gaia":1.0, "Hipparcos":1.0, "Debris":1.0}), 0.0001, "debris"),
    # Debris
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 40000000, 'Earth', 'Solar System'),
          TemporalData(400.0), UI({"Gaia":1.0, "Hipparcos":1.0, "Debris":1.0}), 13.0, "debris2"),
    # Moon
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 2600000, 'Moon', 'Solar System'),
          TemporalData(1.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 26.0, "moon", earthtomoon),
    # Moon
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 2600000, 'Moon', 'Solar System'),
          TemporalData(1.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 20.0, "moon2", moon),
    # Phobos Zoom-in
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 3000000, 'Phobos', 'Solar System'),
          TemporalData(1.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 20.0, "phobosbeg", moontophobos),
    # Phobos
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 30000, 'Phobos', 'Solar System'),
          TemporalData(500.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 3.0),
    # Phobos
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 30000, 'Phobos', 'Solar System'),
          TemporalData(500.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 30.0, "phobos", phobosorbit),
    # Phobos
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 300000000, 'Phobos', 'Solar System'),
          TemporalData(500.0), UI({"Gaia":1.0, "Hipparcos":1.0}), 11.0, "phobosend", phobosend),
    # Voyager 2
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 10, 'Voyager 2', 'Solar System'),
          TemporalData(1.0, jupitertime), UI({"Gaia":1.0, "Hipparcos":1.0}), 4.25, "jupiterbeg1", phobosToVoyager),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 10, 'Voyager 2', 'Solar System'),
          TemporalData(1.0, jupitertime2), UI({"Gaia":1.0, "Hipparcos":1.0}), 20.0, "jupiterend", voyagerCamera),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 10, 'Voyager 2', 'Solar System'),
          TemporalData(1.0, saturntime), UI({"Gaia":1.0, "Hipparcos":1.0}), 6.25, "saturnbeg", voyagerCamera),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 10, 'Voyager 2', 'Solar System'),
          TemporalData(1.0, saturntime2), UI({"Gaia":1.0, "Hipparcos":1.0}), 20.0, "saturnend", voyagerCamera),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 10, 'Voyager 2', 'Solar System'),
          TemporalData(1.0, uranustime), UI({"Gaia":1.0, "Hipparcos":1.0}), 6.25, "uranusbeg", voyagerCamera),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 10, 'Voyager 2', 'Solar System'),
          TemporalData(1.0, uranustime2), UI({"Gaia":1.0, "Hipparcos":1.0}), 20.0, "uranusend", voyagerCamera),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 10, 'Voyager 2', 'Solar System'),
          TemporalData(1.0, neptunetime), UI({"Gaia":1.0, "Hipparcos":1.0}), 6.25, "neptunebeg", voyagerCamera),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 10, 'Voyager 2', 'Solar System'),
          TemporalData(1.0, neptunetime2), UI({"Gaia":1.0, "Hipparcos":1.0}), 20.0, "neptuneend", voyagerCamera),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1.6e+13, 'Voyager 2', 'Solar System'),
          TemporalData(1.0, now), UI({"Gaia":1.0, "Hipparcos":1.0, "Orbits":1.0, "PlanetsLabels":1.0}), 10.0, "voyagerend", voyagerToSS1),
    # Outer Solar System
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1.6e+13, 'Sun', 'Solar System'),
        TemporalData(6470000.0), UI({"Gaia":1.0, "Hipparcos":1.0, "Orbits":1.0, "PlanetsLabels":1.0}), 20.0, "outer1", voyagerToSS2),
    # Outer Solar System
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1.6e+13, 'Sun', 'Solar System'),
        TemporalData(6470000.0), UI({"Gaia":1.0, "Hipparcos":1.0, "Orbits":1.0, "PlanetsLabels":1.0}), 34.0, "outer2", outerSS),
    # Solar System dynamics Constellations
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1.6e+13, 'Sun', 'Solar System'),
        TemporalData(10010000.0), UI({"Gaia":1.0, "Hipparcos":1.0, "Constellations":1.0, "Andromeda":1.0, "M33":1.0, "LG Dwarves":1.0, "Orbits":1.0, "PlanetsLabels":1.0}), 5.0, "ssend", outerSS),
    # Proxima Centauri
    Scene(SpatialData(Vector3(-0.000484189, -0.000821215, -0.000876466), 3.122706524e+9, 'Proxima Centauri', 'Alpha Centauri'),
          TemporalData(10000.0), UI({"Gaia":1.0, "Hipparcos":1.0, "Constellations":1.0, "Andromeda":1.0, "M33":1.0, "LG Dwarves":1.0,"Orbits":1.0, "PlanetsLabels":1.0}), 15.0, "alpCen", alpCen),
    # Proxima Centauri 2
    Scene(SpatialData(Vector3(-0.000484189, -0.000821215, -0.000876466), 3.122706524e+9, 'Proxima Centauri', 'Alpha Centauri'),
          TemporalData(10000.0), UI({"Gaia":1.0, "Hipparcos":1.0, "Constellations":1.0, "Andromeda":1.0, "M33":1.0, "LG Dwarves":1.0,"Orbits":1.0, "PlanetsLabels":1.0}), 37.0, "alpCen2", alpCen2),
    # 51 Peg dynamics
    Scene(SpatialData(Vector3(0.0132362, -0.00132559, 0.00625585), 1.65181e+10, '51 Peg', '51 Peg'),
        TemporalData(10000.0), UI({"Gaia":1.0, "Hipparcos":1.0, "Constellations":1.0, "Andromeda":1.0, "M33":1.0, "LG Dwarves":1.0, "PlanetsLabels":1.0}), 12.0, "51peg", peg51),
    # 51 Peg dynamics 2
    Scene(SpatialData(Vector3(0.0132362, -0.00132559, 0.00625585), 1.65181e+10, '51 Peg', '51 Peg'),
        TemporalData(10000.0), UI({"Gaia":1.0, "Hipparcos":1.0, "Constellations":1.0, "Andromeda":1.0, "M33":1.0, "LG Dwarves":1.0, "PlanetsLabels":1.0}), 30.0, "51peg2", peg512),
    # Radio
    #Scene(SpatialData(Vector3(0.0132362, -0.00132559, 0.00625585), 1.511e+18),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1.511e+18),
        TemporalData(), UI({"Gaia":1.0, "Hipparcos":1.0, "Constellations":1.0, "Andromeda":1.0, "M33":1.0, "LG Dwarves":1.0, "Radio":0.0}), 20.0, "radio", radio),
    # Radio 2
    #Scene(SpatialData(Vector3(0.0132362, -0.00132559, 0.00625585), 1.511e+18),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1.511e+18),
        TemporalData(), UI({"Gaia":1.0, "Hipparcos":1.0, "Constellations":1.0, "Andromeda":1.0, "M33":1.0, "LG Dwarves":1.0, "Radio":1.0, "AnimationTime":0.401}), 29.0, "radio2", radio2),
    # Milky Way
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 6.171e+20),
        TemporalData(), UI({"Dynamic AGORA":1.0, "Andromeda":1.0, "M33":1.0, "LG Dwarves":1.0, "AnimationTime":0.401}), 37.0, "mw", radioToMW),
    # Milky Way + exoplanets
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 6.171e+20),
        TemporalData(), UI({"Dynamic AGORA":1.0, "Exoplanets":0.001, "Andromeda":1.0, "M33":1.0, "LG Dwarves":1.0, "AnimationTime":0.401}), 35.0, "mwexo", mwpart1),
    # Milky Way
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 6.171e+20),
        TemporalData(), UI({"Dynamic AGORA":1.0, "Exoplanets":0.0, "Andromeda":1.0, "M33":1.0, "LG Dwarves":1.0, "AnimationTime":0.401}), 32.0, "mwevolbeg", mwpart2),
    # Milky Way Evolution
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 6.171e+20),
        TemporalData(), UI({"Dynamic AGORA":1.0, "Andromeda":1.0, "M33":1.0, "LG Dwarves":1.0, "AnimationTime":1.0}), 72.0, "mwevolend", mwdyn),
    # Local Group part 1
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 2.0e+22),
            TemporalData(), UI({"Labels2":1.0, "Volumetric AGORA":1.0, "Andromeda":2.0, "M33":2.0, "LG Dwarves":5.0, "AnimationTime":1.0}), 50.0, "lg1", lg1),
    # Local Group part 2
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 3.04e+22),
        TemporalData(), UI({"Volumetric AGORA":1.0, "Andromeda":2.0, "M33":2.0, "LG Dwarves":5.0, "AnimationTime":1.0}), 25.0, "lg2", lg2),
    # Local Group reset animtime
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 3.04e+22),
        TemporalData(), UI({"Volumetric AGORA":1.0, "Andromeda":2.0, "M33":2.0, "LG Dwarves":5.0, "AnimationTime":0.0}), 0.1),
    # Milky Way/Andromeda Collision
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 3.04e+22),
        TemporalData(), UI({"MWAndr":1.0, "AnimationTime":0.0}), 1.9, "mwandrbeg"),
    # Milky Way/Andromeda Collision
    Scene(SpatialData(Vector3(81.0, -182.0, -40.0), 4.0e+21),
        TemporalData(), UI({"MWAndr":1.0, "AnimationTime":1.0}), 56.0, "mwandr", mwandr),
    # Milky Way/Andromeda Collision
    Scene(SpatialData(Vector3(81.0, -182.0, -40.0), 4.0e+21),
        TemporalData(), UI({"MWAndrEnd":1.0, "AnimationTime":1.0}), 0.2, "mwandrend"),
    # Milky Way/Andromeda Collision
    Scene(SpatialData(Vector3(81.0, -182.0, -40.0), 4.0e+21),
        TemporalData(), UI({"MWAndrEnd":1.0, "AnimationTime":0.0}), 0.8, "mwandrend2"),
    # Galaxy Cluster Animation beg
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 2.1e+23),
        TemporalData(), UI({"ClusterDynamics":0.5, "AnimationTime":0.5}), 3.0, "clusterdyn"),
    # Galaxy Cluster Animation ~ 25sec
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 2.1e+23),
        TemporalData(), UI({"ClusterDynamics":0.5, "AnimationTime":1.0}), 36.0, "clusterdynend"),
    # Illustris
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 2.0e+24),
        TemporalData(), UI({"IllustrisTNG":1.0, "AnimationTime":1.0}), 68.0, "illustris"),
    # SDSS close
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 2.0e+25),
        TemporalData(), UI({"SDSS":1.0}), 39.75, "sdss", sdss1),
    # SDSS distant
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 1.0e+26),
        TemporalData(), UI({"SDSS":1.0}), 72.875, "sdss2", sdss2),
    # CMB
    Scene(SpatialData(Vector3(-0.43, -8.24, -0.81), 4.0e+26),
        TemporalData(), UI({"SDSS":1.0, "CMB":1.0}), 46.375, "cmb", cmb),
    # Small portion of CMB with Cube ready to evolve
    Scene(SpatialData(Vector3(0.0, -13946899, 0.0), 6.0e+23),
        TemporalData(), UI({"SDSS":1.0, "CMB":0.4, "CosmoDynamics":1.0, "AnimationTime":0.0}), 10.0, "cubeevolstart", cubeevolstart),
    Scene(SpatialData(Vector3(0.0, -13946899, 0.0), 6.0e+23),
        TemporalData(), UI({"SDSS":1.0, "CMB":0.4, "CosmoDynamics":1.0, "AnimationTime":0.0}), 10.0, "cubeevolstart2", cubeevolstart2),
    # Cube Evolution
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 8.0e+22),
        TemporalData(), UI({"Volumetric AGORA":1.0, "Andromeda":2.0, "M33":2.0, "LG Dwarves":10.0, "CosmoDynamics":0.5, "CMB":0.4, "AnimationTime":1.0}), 30.0, "cubeevolend", cosmodyn),
    # Local Group
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 3.04e+22),
        TemporalData(1.0, endtime), UI({"Volumetric AGORA":1.0, "Andromeda":2.0, "M33":2.0, "LG Dwarves":10.0, "AnimationTime":1.0}), 15.0, "lgback", lgback),
    # MW
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 6.171e+20),
        TemporalData(1.0, endtime), UI({"Volumetric AGORA":1.0, "Andromeda":2.0, "M33":2.0, "LG Dwarves":10.0, "AnimationTime":1.0}), 15.0, "mwback", mwback),
    # Outer Solar System
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 1.6e+13, 'Earth', 'Solar System'),
        TemporalData(1.0, endtime), UI({"Gaia":1.0, "Hipparcos":1.0, "Orbits":1.0, "PlanetsLabels":1.0}), 10.0, "ssback", ssback),
    # Earth
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 12000000, 'Earth', 'Solar System'),
            TemporalData(1.0, endtime), UI({"Gaia": 1.0, "Hipparcos":1.0}), 20.0, "earthback", earthback),
    # Earth
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 12000000, 'Earth', 'Solar System'),
            TemporalData(1.0, endtime), UI({"Gaia": 1.0, "Hipparcos":1.0}), 40.0, "end1", end1),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 12000000, 'Earth', 'Solar System'),
            TemporalData(1.0, endtime), UI({"Gaia": 1.0, "Hipparcos":1.0}), 10.0, "end2", end2),
    # Credits
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 8.0e+22),
            TemporalData(1.0, endtime), UI({"Credits0": 0.0}), 1.0, "credits", black),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 8.0e+22),
            TemporalData(1.0, endtime), UI({"Credits0": 1.0}), 5.0, "credits0"),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 8.0e+22),
            TemporalData(1.0, endtime), UI({"Credits1": 1.0}), 5.0, "credits1"),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 8.0e+22),
            TemporalData(1.0, endtime), UI({"Credits2": 1.0}), 5.0, "credits2"),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 8.0e+22),
            TemporalData(1.0, endtime), UI({"Credits3": 1.0}), 5.0, "credits3"),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 8.0e+22),
            TemporalData(1.0, endtime), UI({"Credits4": 1.0}), 5.0, "credits4"),
    Scene(SpatialData(Vector3(0.0, 0.0, 0.0), 8.0e+22),
            TemporalData(1.0, endtime), UI({"Credits5": 1.0}), 5.0, "credits5"),
]

### COMOV DISTANCES FOR COSOMODYNAMICS
#comovdistfile="/media/florian/Archive/CosmoDynamics/COMOV_DISTANCES"
#with open(comovdistfile, "r") as f:
#    content=f.readlines()

comovdists=[]
#for c in content:
#    comovdists.append(float(c.split('\t')[1]))

def getAnimationTimeFromComovDist(comov_dist):
    global comovdists
    for i in range(len(comovdists)):
        if comovdists[i] > comov_dist:
            continue
        if i == 0:
            return 0.0
        index = i - ((comov_dist - comovdists[i]) / (comovdists[i-1] - comovdists[i]))
        return index / (len(comovdists)-1)

### END COMOV DISTANCES FOR COSOMODYNAMICS

def getIdFromName(name):
    global scenes

    for i in range(len(scenes)):
        if scenes[i].name == name:
            return i

if VIRUP.videomode:
    id = 0
    #id = getIdFromName("mwevolbeg") #!IDSTART
    autoplaymovie=True
else:
    id = 0
    #id = getIdFromName("mwexo")
    autoplaymovie=True


cubeevolstartid=getIdFromName("cubeevolstart2")
jupiterendid=getIdFromName("jupiterend")

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
            return Vector3(cos(shiftvertangle)*cos(shiftangle)*val, cos(shiftvertangle)*sin(shiftangle)*val, sin(shiftvertangle)* val)
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
            return Vector3(cos(shiftvertangle)*cos(shiftangle)*val, cos(shiftvertangle)*sin(shiftangle)*val, sin(shiftvertangle)* val)
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
            currentscene=Scene(SpatialData(Universe.cosmoPosition - getCosmoShift(), 1.0 / Universe.scale, Universe.planetTarget, Universe.planetarySystemName),
               TemporalData(Universe.timeCoeff, Universe.simulationTime), scenes[oldid].ui)
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

    Universe.simulationTime = isspassdt
    timer = Timer()
    longanimation = False
    currentscene = None

    totaltime=0
    for scene in scenes:
        totaltime += scene.transitiontimeto
    print(totaltime / 60.0)
    print(totaltime)

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

    #if id == getIdFromName("mwevolbeg"):
    #    VIRUP.close()

    """
    if id <= cubeevolstartid-3:
        Universe.setSolarSystemPosition("CosmoDynamics", Vector3(10000,10000,10000) - Vector3(-1000.0, -800.0, 2500.0))
    elif id <= cubeevolstartid:
        Universe.setSolarSystemPosition("CosmoDynamics", Vector3(10000,10000,10000) - Vector3(0.0, -13946899, 0.0))
    else:
        Universe.setSolarSystemPosition("CosmoDynamics", Vector3(10000,10000,10000))
    """

    t_harsh = timer.elapsed() / ((scenes[id].transitiontimeto) * 1000.0)
    t = smoothstep(t_harsh)
    nextid = -1
    if t <= 1.0 and t >= 0.0 and currentscene != None:
        scene=interpolateScene(currentscene, scenes[id], t)
        if id == cubeevolstartid+1:
            Universe.setSolarSystemPosition("CosmoDynamics", Vector3(10000,10000,10000) - scene.spatialData.cosmoPos)
    else:
        t_harsh = 1.0
        t = 1.0
        if currentscene != None:
            scene=interpolateScene(currentscene, scenes[id], 1.0)
        else:
            scene=scenes[id]
        nextid = id+1

    spatialData = scene.spatialData
    Universe.scale = spatialData.scale
    if spatialData.systemName != "" and spatialData.bodyName != "" and Universe.planetarySystemLoaded:
        if Universe.planetarySystemName == spatialData.systemName:
            Universe.planetTarget = spatialData.bodyName
            Universe.planetPosition = spatialData.planetPos
        else:
            Universe.cosmoPosition = Universe.getSystemAbsolutePosition(systemName)
    else:
        Universe.cosmoPosition = spatialData.cosmoPos

    temporalData = scene.temporalData
    Universe.timeCoeff = temporalData.timeCoeff
    if temporalData.simulationTime != None:
        if temporalData.simulationTime.isValid() and t <= 1:
            Universe.simulationTime = temporalData.simulationTime

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

    debris=ui.getLum("Debris")
    animationtime=ui.getLum("AnimationTime")
    #if debris > 0.5 and Universe.videomode:
    #    Universe.setRenderSpacecrafts(True)
    #else:
    #    Universe.setRenderSpacecrafts(False)
    Universe.setAnimationTime(animationtime)

    Universe.camYaw = shiftangle
    Universe.camPitch = -shiftvertangle
    shiftangle = 0.0
    shiftvertangle = 0.05

    # apply custom function
    scenes[id].custom(t, t_harsh)
    ToneMappingModel.exposure *= fade_factor

    """
    print(id)
    print(Universe.camYaw)
    print(Universe.camPitch)
    print(Universe.planetPosition)
    print(Universe.planetTarget)
    print("")
    """
    if nextid != -1 and nextid < len(scenes) and autoplaymovie:
        setSceneId(nextid)

    if spatialData.bodyName != '' and Universe.planetarySystemLoaded:
        Universe.planetPosition += getPlanetShift()
    else:
        Universe.cosmoPosition += getCosmoShift()

    # Synra dome
    #if Universe.projection == "domemaster180":
    #    Universe.camPitch += 45 * pi / 180
