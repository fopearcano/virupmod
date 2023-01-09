from ftplib import FTP
import os

def decodeFtpUrl(url):
    if url[0:6] != "ftp://":
        raise(BaseException("Url doesn't start by ftp://"))
    beg=6
    end=url.find('/', beg)
    domain=url[beg:end]

    beg=end+1
    end=url.rfind('/')
    directory=url[beg:end]

    beg=end+1
    file=url[beg:]

    return (domain, directory, file)

totsize=0

def getfilesize(url):
    global totsize

    [domain, directory, file] = decodeFtpUrl(url)
    with FTP(domain) as ftp:
        ftp.login()
        ftp.cwd(directory)
        ftp.sendcmd("TYPE i")
        totsize=ftp.size(file)
        return totsize

def downloadFtp(url, destfile, resume=True):
    global downloadRunning
    global totsize

    [domain, directory, file] = decodeFtpUrl(url)

    with FTP(domain) as ftp:
        ftp.login()
        ftp.cwd(directory)
        ftp.sendcmd("TYPE i")
        with open(destfile, 'ab') as fp:
            totsize=ftp.size(file)
            size=os.stat(destfile).st_size
            ftp.retrbinary('RETR ' + file, fp.write, rest=str(size))

import zipfile

total_files_number=0
currentfileindex=-1
currentfilepath=""
currentfiletotsize=0

def getzipfilesize(zippath):
    global totsize

    totsize=0
    z = zipfile.ZipFile(zippath)
    for f in z.filelist:
        totsize += f.file_size
    return totsize

def getzipfilesnumber(zippath):
    global total_files_number

    total_files_number = len(zipfile.ZipFile(zippath).filelist)
    return total_files_number

def resetextraction():
    global total_files_number
    global currentfileindex
    global currentfilepath
    global currentfiletotsize

    total_files_number = 0
    currentfileindex=-1
    currentfilepath = ""
    currentfiletotsize=0

def preparenextextraction(zippath):
    global currentfileindex
    global currentfilepath
    global currentfiletotsize

    currentfileindex += 1
    fz = zipfile.ZipFile(zippath).filelist[currentfileindex]
    currentfilepath=fz.filename
    currentfiletotsize=fz.file_size

def extract(zippath):
    if currentfilepath[-1] == '/':
        return
    end=zippath.rfind('/')
    directory=zippath[:end]
    os.chdir(directory)

    end=currentfilepath.rfind('/')
    os.makedirs(currentfilepath[:end], exist_ok=True)

    z = zipfile.ZipFile(zippath)
    z.extract(currentfilepath)

