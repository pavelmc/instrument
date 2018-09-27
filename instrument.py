###########################################################################
# Multi-instrumento
#
# Author: Pavel Milanes Costa
# Email: pavelmc@gmail.com
#
###########################################################################

import sys
import os
import traceback
import math
import time
import serial
import serial.tools.list_ports as serialdevs

# QT5
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *

# Matplotlib
from numpy import arange, sin, pi, random
import matplotlib
matplotlib.use("Qt5Agg")
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt5 import NavigationToolbar2QT as NavigationToolbar
from matplotlib.figure import Figure

progname = os.path.basename(sys.argv[0])
progversion = "0.1"

# change this to the appropriate serial port for your setup
SERIALPORT_SPEED = 115200
SERIALPORT_TIMEOUT = 0.07

sampleList = [50, 100, 200, 500, 1000, 2000, 4000, 8000] # samples per sweep

# limits
min_start = 100000      # 100 kHz
max_stop  = 200100000   # 220 MHz

# Main vars
runMode = 0     # 0 = stoppped, 1 = running

# Trieger to simulate a serial device
simserial = False

# get a list of valid ports
def get_serial_ports():
    ports = []
    # get the list of ports on this system, with a description
    for port in serialdevs.comports():
        ports.append([port[0],port[1]])

    # fail safe for no valid ports: we need to show something
    # in this case default fake ports
    if len(ports) == 0:
        if os.name == "nt":
            ports.append(["COM1", "Fake port"])
        else:
            ports.append(["/dev/ttyS0", "Fake port"])

    return ports


# capturing click events in mpl
def onclick(event):
        px, py = event.xdata, event.ydata
        print("x = %d, y = %d" % (px, py))

        #~ global coords
        #~ coords.append((ix, iy))

        #~ if len(coords) == 2:
            #~ fig.canvas.mpl_disconnect(cid)

        #~ return coords


# signals to emit during a scan
class WorkerSignals(QObject):
    finished = pyqtSignal()
    error = pyqtSignal(tuple)
    result = pyqtSignal(int)
    progress = pyqtSignal(int)


# define a worker that cals a function of the parent object.
class Worker(QRunnable):

    def __init__(self, fn, *args, **kwargs):
        super(Worker, self).__init__()
        # Store constructor arguments (re-used for processing)
        self.fn = fn
        self.args = args
        self.kwargs = kwargs
        self.signals = WorkerSignals()

        # Add the callback to our kwargs
        kwargs['progress_callback'] = self.signals.progress


    @pyqtSlot()
    def run(self):
        try:
            # the thing to do
            result = self.fn(*self.args, **self.kwargs)
        except:
            # report the error back
            traceback.print_exc()
            exctype, value = sys.exc_info()[:2]
            self.signals.error.emit((exctype, value, traceback.format_exc()))
        else:
            # Return the result of the processing
            self.signals.result.emit(result)
        finally:
            # Done
            self.signals.finished.emit()


# special canvas class
class MyMplCanvas(FigureCanvas):
    """Ultimately, this is a QWidget (as well as a FigureCanvasAgg, etc.)."""

    def __init__(self, parent=None, width=5, height=4, dpi=150):
        fig = Figure(figsize=(width, height), dpi=dpi)
        self.axes = fig.add_subplot(111)

        FigureCanvas.__init__(self, fig)
        self.setParent(parent)

        FigureCanvas.setSizePolicy(self,
                                   QSizePolicy.Expanding,
                                   QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)


# canvas to draw the graph
class MyDynamicMplCanvas(MyMplCanvas):
    """A canvas that updates itself every second with a new plot."""

    # create it
    def __init__(self, *args, **kwargs):
        MyMplCanvas.__init__(self, *args, **kwargs)
        timer = QTimer(self)
        timer.timeout.connect(self.update_it)
        timer.start(50)

        # put some data in the data arrays
        self.x = arange(0, 1.0, 0.02)
        self.y = arange(0, 1.0, 0.02)

        #create the axes
        self.axes.plot(self.x, self.y)
        # this will hold the axes span to calc new limits
        self.limits = list("0000")

        # coordinates format
        self.axes.format_coord = self.coord_format

        # draw the graph, modified flag
        self.modified = True
        self.update_it()


    # update it
    def update_it(self):
        # check if its has been modified
        if (self.modified == False):
            return

        # then update the status of the graph

        # clear the axes, or it will draw one over the other
        self.axes.cla()

        # set the data for the plot
        self.axes.plot(self.x, self.y)

        # title and labels
        self.axes.set_title('Sweep Freq vs. level')
        self.axes.set_ylabel('Level in dB')
        self.axes.set_xlabel('Frequency in Mhz')

        # turn on grid
        self.axes.grid(True)

        # set the axes limits
        self.limit_axes()

        # draw the data
        self.draw()

        # clean modified flag
        self.modified = False


    # calc ranges and limits of the plots and set the limits plus a x %
    def limit_axes(self):
        # [xmin, xmax,ymin,ymax]
        self.limits[0] = min(self.x)
        self.limits[1] = max(self.x)
        self.limits[2] = min(self.y)
        self.limits[3] = max(self.y)

        # size or span
        x_size = abs(self.limits[1] - self.limits[0])
        y_size = abs(self.limits[3] - self.limits[2])

        # amount to strech beyond real limits
        border = 2  # in %
        xb = (x_size * border) / 100.0
        yb = (y_size * border) / 100.0

        # mode limits
        self.limits[0] = self.limits[0] - xb
        self.limits[1] = self.limits[1] + xb
        self.limits[2] = self.limits[2] - yb
        self.limits[3] = self.limits[3] + yb

        # set it
        self.axes.set_xlim(self.limits[0], self.limits[1])
        self.axes.set_ylim(self.limits[2], self.limits[3])


    # format the position in the coordinates
    # to set the real y value, not the position
    def coord_format(self, x, y):
        # ident index to know y value
        return "%.6f MHz (%.3f dB)" % (x, y)


# main app
class ApplicationWindow(QMainWindow):

    # initializer function
    # Basically draw the interface and init the vars
    def __init__(self):
        QMainWindow.__init__(self)
        self.setAttribute(Qt.WA_DeleteOnClose)
        self.setWindowTitle("application main window")

        self.file_menu = QMenu('&File', self)
        self.file_menu.addAction('&Open', self.fileOpen,
                                 Qt.CTRL + Qt.Key_O)
        self.file_menu.addAction('&Save', self.fileSave,
                                 Qt.CTRL + Qt.Key_S)
        self.file_menu.addAction('&Quit', self.fileQuit,
                                 Qt.CTRL + Qt.Key_Q)

        self.menuBar().addMenu(self.file_menu)

        self.help_menu = QMenu('&Help', self)
        self.menuBar().addSeparator()
        self.menuBar().addMenu(self.help_menu)

        self.help_menu.addAction('&About', self.about)

        self.main_widget = QWidget(self)

        # main layout
        mainLayout = QVBoxLayout(self.main_widget)

        # add the main plot
        self.graph = MyDynamicMplCanvas(self.main_widget, width=5, height=4, dpi=100)
        self.tbox = NavigationToolbar(self.graph, self)
        plot = QVBoxLayout()
        plot.addWidget(self.graph)
        plot.addWidget(self.tbox)

        # add it to the main layout
        mainLayout.addLayout(plot)

        # adding the input bar layout
        inputBar = QHBoxLayout()
        # labels
        ldevice = QLabel("Device:")
        lstart = QLabel("Start Freq:")
        lstop = QLabel("Stop Freq:")
        lsampling = QLabel("Sampling:")
        laverage = QLabel("Average:")

        # combo boxes
        # device and populate it
        self.avail_serial = get_serial_ports()
        self.cbdevice = QComboBox()
        for (p, pdesc) in self.avail_serial:
            self.cbdevice.addItem("%s (%s)" % (p, pdesc))

        # samples and populate it
        self.cbsamples = QComboBox()
        for sample in sampleList:
            self.cbsamples.addItem(str(sample))

        # set default index to 500 samples
        self.cbsamples.setCurrentIndex(3)

        # average and populate it
        self.cbaverage = QComboBox()
        for ave in range(1, 11, 1):
            self.cbaverage.addItem(str(ave))

        # text lines
        self.startFreq = QLineEdit("", width=5)
        self.stopFreq = QLineEdit("", width=5)
        # will grab limits in each case
        self.set_start_line(0)
        self.set_stop_line(0)

        # Adding it
        inputBar.addWidget(ldevice)
        inputBar.addWidget(self.cbdevice)
        inputBar.addWidget(lstart)
        inputBar.addWidget(self.startFreq)
        inputBar.addWidget(lstop)
        inputBar.addWidget(self.stopFreq)
        inputBar.addWidget(lsampling)
        inputBar.addWidget(self.cbsamples)
        inputBar.addWidget(laverage)
        inputBar.addWidget(self.cbaverage)
        # stretcher
        inputBar.addStretch()

        # add it to the main layout
        mainLayout.addLayout(inputBar)

        # adding the button bar layout
        buttons = QHBoxLayout()
        # start
        bStart = QPushButton(text="Start")
        #bStart.setCheckable(True)
        bStop = QPushButton(text="Stop")
        self.bRef = QPushButton(text="Set Reference")

        # create a "Continuous scan" checkbox
        self.cs = QCheckBox("Continuous Scan")

        # create a "Carrier On" checkbox
        self.co = QCheckBox("Carrier On")
        self.co.setChecked(True)

        # create progress bar
        self.pbar = QProgressBar()
        self.pbar.isTextVisible = True
        self.pbar.setMaximum(100)
        self.pbar.setMinimum(0)
        self.pbar.setValue(0)

        # Adding it
        buttons.addWidget(bStart)
        buttons.addWidget(bStop)
        buttons.addWidget(self.bRef)
        buttons.addWidget(self.co)
        buttons.addWidget(self.cs)

        # stretcher
        buttons.addStretch()

        # progress bar al the far right
        buttons.addWidget(self.pbar)

        # connect the butttons to acctions
        bStart.clicked.connect(self.scanInit)
        bStop.clicked.connect(self.scanStop)
        self.bRef.clicked.connect(self.set_ref)
        self.cs.stateChanged.connect(self.scan_type_check)
        self.co.stateChanged.connect(self.carrier_on_check)

        # triger some actions on change
        self.cbsamples.activated.connect(self.break_ref)
        self.cbaverage.activated.connect(self.get_ave)

        # add it to the main layout
        mainLayout.addLayout(buttons)

        # Status bar
        self.status = self.statusBar()
        self.status.showMessage("Ready")

        self.main_widget.setFocus()
        self.setCentralWidget(self.main_widget)

        # thread model
        self.threadpool = QThreadPool()

        # def main vars used by the sweep
        self.runMode = 0
        self.continuous = False
        self.average = 1
        self.ref = []
        self.refActive = False
        self.scanCompleted = False
        self.scanStoped = False
        self.carrier = True

        # update some vars
        self.update_scan_params()

        # serial opened flag
        self.workingSerial = False


    # clean the serial buffer
    def serial_clean(self):
        count = 0
        data = "sample"
        while (len(data) > 0):
            data = self.serial.read()
            count += 1
            if (count > 10):
                # TODO: rise error
                break


    # check the scan type: single or continuous
    def scan_type_check(self):
        if (self.cs.isChecked()):
            # it's checked
            self.continuous = True
        else:
            # not checked
            self.continuous = False


    # check if Carrier must be on during scan
    def carrier_on_check(self):
        if (self.co.isChecked()):
            # it's checked
            self.carrier = True
        else:
            # not checked
            self.carrier = False


    # set the value in the start freq lineEdit
    def set_start_line(self, val = 0):
        # val is an integer in Hz

        # limit checks
        if (val == 0 or (val < min_start or val > max_stop)):
            val = min_start

        # Scale to MHz and assigns
        temp = val/1000000.0
        self.startFreq.setText(str(temp))


    # set the value in the stop freq lineEdit
    def set_stop_line(self, val = 0):
        # val is an integer in Hz

        # limit checks
        if (val == 0 or (val < min_start or val > max_stop)):
            val = max_stop

        # Scale to MHz and assigns
        temp = val/1000000.0
        self.stopFreq.setText(str(temp))


    # get the start and stop from the text boxes
    def get_start_stop(self):
        temp = str(self.startFreq.text())
        start = int(float(temp) * 1000000)

        # limit lock
        if (start < min_start):
            start = min_start
            self.set_start_line(start)

        temp = str(self.stopFreq.text())
        stop = int(float(temp) * 1000000)

        # limit lock
        if (stop > max_stop):
            stop = max_stop
            self.set_stop_line(stop)


        return [start, stop]


    # get the samples in the combo box
    def get_samples(self):
        samples = sampleList[self.cbsamples.currentIndex()]
        return samples


    # get the average in the combo box
    def get_ave(self):
        # the index is the value -1, so, 0 = 1
        self.average = self.cbaverage.currentIndex() + 1


    # set a msg on the status bar
    def set_status_msg(self, msg, count=0):
        # check if we will put a timeout in ms
        if (count == 0):
            # no timeout
            self.status.showMessage(msg)
        else:
            # check if bigger than permitted
            if (count > 5000):
                #reset it
                count = 5000

            # set it with timeout
            self.status.showMessage(msg, count)


    # stop the scan
    def scanStop(self):
        self.runMode = 0
        self.scanStoped = True
        self.cs.setChecked(False)


    # make a sweep, no matter if single shot or continuous
    def sweep(self, progress_callback):
        index = 0
        f = self.start
        dbm = 0.0

        # clean serial buffer
        self.serial_clean()

        # default return char
        ret = str("\n").encode()

        # send the carrier Enable or Disable command
        if (self.carrier == True):
            setf = str("e").encode()
            print("Send Carrier Enable")
        else:
            setf = str("d").encode()
            print("Send Carrier Disable")

        # send the command
        self.serial.write(setf + ret)
        # time to process
        time.sleep(0.01)
        # force a freq send and a line read to apply and clean the buffer
        setf = str(f).encode()
        self.serial.write(setf + ret)
        # time to process
        time.sleep(0.01)
        # now read a line to clean de buffer
        line = self.serial.readline()

        while(index < len(self.graph.x)):
            #check if run mode has changed to break the loop
            if (self.runMode == 0):
                self.set_status_msg("Scan stoped by user request")
                break

            # set freq on device
            # python 3 handle all strings as unicode
            setf = str(f).encode()

            # averaging
            avec = 0
            val = 0
            while (avec < self.average):
                # request the data
                self.serial.write(setf + ret)

                # get it
                line = self.serial.readline()

                # remove spaces
                line = line.strip()

                # conver to integer
                if len(line) > 0:
                    line = float(line) / 100.0
                else:
                    line = 0.0

                # DEBUG
                #~ print ("Result f: %i: %f" % (f, line))

                # add it + increment counter
                val += line
                avec += 1

            # average readings
            dbm = val / self.average

            # update data
            if (self.refActive):
                # difference
                ref = round(self.ref[index], 3) * -1.0
                self.graph.y[index] = ref + round(dbm, 3)
            else:
                # normal way
                self.graph.y[index] = round(dbm, 3)


            # increment counter and freq
            index = index + 1
            f = f + self.step

            # flag update live data
            self.graph.modified = True

            # callback to update progress bar
            progress_callback.emit(index)


    # update the progress in the UI
    def progress(self, index):
        # overflow handling
        if (index >= self.samples):
            # set progress bar to 100
            self.pbar.setValue(100)
        else:
            # set progress bar value
            self.pbar.setValue((100 * index)/self.samples)

        # keep the UI responsive during scan
        qApp.processEvents()


    # update scan parameters from the UI
    def update_scan_params(self):
        # calcs
        self.start, self.stop = self.get_start_stop()
        self.samples = self.get_samples()
        self.step = int((self.stop - self.start) / self.samples)
        self.get_ave()

        # set ranges to correct values for the scale.
        s = self.start / 1000000.0
        e = self.stop  / 1000000.0
        p = self.step  / 1000000.0

        # set the correct dimension
        self.graph.x = arange(s, e, p)
        self.graph.y = arange(s, e, p)

        # reset range y to zero
        dim = len(self.graph.x)
        for i in range(dim):
            self.graph.y[i] = 0.0

        # update the reference if needed
        if (not self.refActive):
            self.ref = list(self.graph.y)


    # init the scan
    def scanInit(self):
        # check if serial is open
        if (self.workingSerial == False):
            # buffer to store parameters from the arduino
            rxbuff= ""

            try:
                # must open serial port
                self.serial = serial.Serial(
                            self.avail_serial[
                            self.cbdevice.currentIndex()][0],
                            SERIALPORT_SPEED,
                            timeout=SERIALPORT_TIMEOUT)

                # arduino boot time, around 5 seconds
                self.set_status_msg("Please wait while Arduino reboots", 4000)

                # responsive wait: 5 seconds
                for a in range(500):
                    # scaled wait
                    time.sleep(0.01)
                    # process events
                    qApp.processEvents()

                # fmin & fmax ~20 chars on the buffer
                rxbuff = self.serial.read(22)
                # parse and set
                self.set_limits(rxbuff)

                # Send the init commands to the arduino
                ret = str("\n").encode()
                init = str("S").encode()
                # init dance
                self.serial.write(ret)
                self.serial.write(init)
                self.serial.write(ret)

                # responsive wait: 1 seconds
                for a in range(100):
                    # scaled wait
                    time.sleep(0.01)
                    # process events
                    qApp.processEvents()

                # now we are talking
                self.workingSerial = True

            except Exception:
                self.set_status_msg("Error! Serial port can't be opened")
                raise
                return

        # run the thread, but only one
        hmany = self.threadpool.activeThreadCount()
        if (hmany < 1):
            # initial set and calcs
            self.update_scan_params()

            # set step comments
            self.set_status_msg("Scan step is %.3f kHz" % (self.step / 1000.0))

            # set the run flag
            self.runMode = 1

            # set graph data
            self.graph.modified = True

            # it's safe to run it
            self.start_thread()
        else:
            self.set_status_msg("A worker is busy, please wait.")


    # set the min and max f values fro the string that the arduino gives
    def set_limits(self, rx):
        # buffer is something like this
        # b'-L100000-H198000000-'
        rxbuff = str(rx)

        # extract the info
        for e in rxbuff.split("-"):
            # lower limit
            if len(e) > 5 and e[0] == "L":
                min_start = int(e[1:])

            # higher limit
            if len(e) > 5 and e[0] == "H":
                max_stop = int(e[1:])

        # print to know
        print("Freq limits reported by arduino is:")
        print("Start: %f MHz, Stop: %f MHz" %
            (float(min_start) / 1000000,
             float(max_stop) / 1000000))

        # update internal and UI
        self.set_start_line(min_start)
        self.set_stop_line(max_stop)
        self.get_start_stop()


    # check if must stop the scan or keep it running
    def check_end_of_scan(self):
        # set progress bar to 100%
        self.progress(10000)

        # set the flasg of scan finished if normally ended
        if (self.scanStoped == True):
            self.scanCompleted = False
        else:
            self.scanCompleted = True

        # continuous scan?
        if (self.continuous == True):
            # restart the worker
            self.start_thread()


    # start a serial thread to read data
    def start_thread(self):
        self.worker = Worker(self.sweep)
        self.worker.signals.progress.connect(self.progress)
        self.worker.signals.finished.connect(self.check_end_of_scan)

        # reset stoped flag
        self.scanStoped = False

        # set flag as we will start a scan
        self.scanCompleted = False

        # init worker
        self.threadpool.start(self.worker)


    # break the ref compatibility due to changes in the UI
    def break_ref(self):
        # break the ability to use a reference
        self.scanCompleted = False
        self.refActive = False
        self.bRef.setText("Set Reference")


    # set reference
    def set_ref(self):
        # process and take/left the reference
        if (self.refActive == True):
            # deactivate
            self.refActive = False
            self.bRef.setText("Set Reference")
        else:
            # activate if possible
            if (self.scanCompleted == True):
                # ok, doit.
                self.refActive = True
                self.bRef.setText("Del Reference")

                # make a copy of the Y range in the local ref mode
                self.ref = list(self.graph.y)
            else:
                # no, you don't have a scan profile
                self.set_status_msg("Not Possible, make a complete scan as reference")


    # quit the gui
    def fileQuit(self):
        self.close()

    # save the data of this scan
    def fileSave(self):
        fname = ""
        formats = ["*.isf"]
        fname = QFileDialog.getSaveFileName(self,
            "Open saved scan", fname, "Scan data files (%s)" % " ".join(formats))

        # fname is a tuple, multiple files are not allowed
        # selecting just first
        fname = str(fname[0])

        if fname:
            if "." not in fname:
                fname += ".isf"

            print(fname)
            #~ self.Save(fname)

    # save the data of this scan
    def fileOpen(self):
        fname = ""
        formats = ["*.isf"]
        fname = QFileDialog.getOpenFileName(self,
            "Save scan data", fname, "Scan data files (%s)" % " ".join(formats))

        # fname is a tuple, multiple files are not allowed
        # selecting just first
        fname = str(fname[0])

        if fname:
            if "." not in fname:
                fname += ".isf"

            print(fname)
            #~ self.Load(fname)

    # close event
    def closeEvent(self, ce):
        self.fileQuit()


    # about box
    def about(self):
        QMessageBox.about(self, "About", \
"""GUI for the Instrument hardware
Developed by Pavel Milanes (CO7WT)
Based on an Matplotlib example for QT4

Any comments or bug to:
pavelmc@gmail.com

73, Happy Hacking/Homebrewing
""")


app = QApplication(sys.argv)
app.setOrganizationName("CO7WT Soft")
app.setOrganizationDomain("co7wt.blogger.com")
app.setApplicationName("Intrument PC interface")
#~ app.setWindowIcon(QIcon(":/icon.png"))

aw = ApplicationWindow()
aw.setWindowTitle("%s" % "Intrument PC interface")
aw.show()
sys.exit(app.exec_())
