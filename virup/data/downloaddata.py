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
    workingdir=os.path.abspath(os.getcwd())
    os.chdir(directory)

    end=currentfilepath.rfind('/')
    os.makedirs(currentfilepath[:end], exist_ok=True)

    z = zipfile.ZipFile(zippath)
    z.extract(currentfilepath)
    os.chdir(workingdir) # reset working dir

