import serial
from PyQt5.QtCore import QTimer
from PyQt5.QtWidgets import QMainWindow, QPushButton, QFileDialog, QLabel, QRadioButton, QMessageBox, QComboBox, \
    QProgressBar
from PyQt5.QtWidgets import QApplication
import sys
from PyQt5.QtGui import QImage, QPixmap, QColor
resized_image=None
cal_bin=1
com_open=""
pixel_list =[]
Crc16Tab = [0, 4129, 8258, 12387, 16516, 20645, 24774, 28903, 33032, 37161, 41290, 45419, 49548, 53677, 57806, 61935, 4657, 528, 12915, 8786, 21173, 17044, 29431, 25302,
            37689, 33560, 45947, 41818, 54205, 50076, 62463, 58334, 9314, 13379, 1056, 5121, 25830, 29895, 17572, 21637, 42346, 46411, 34088, 38153, 58862, 62927, 50604, 54669, 13907,
            9842, 5649, 1584, 30423, 26358, 22165, 18100, 46939, 42874, 38681, 34616, 63455, 59390, 55197, 51132, 18628, 22757, 26758, 30887, 2112, 6241, 10242, 14371, 51660, 55789,
            59790, 63919, 35144, 39273, 43274, 47403, 23285, 19156, 31415, 27286, 6769, 2640,14899, 10770, 56317, 52188, 64447, 60318, 39801, 35672, 47931, 43802, 27814, 31879,
            19684, 23749, 11298, 15363, 3168, 7233, 60846, 64911, 52716, 56781, 44330, 48395,36200, 40265, 32407, 28342, 24277, 20212, 15891, 11826, 7761, 3696, 65439, 61374,
            57309, 53244, 48923, 44858, 40793, 36728, 37256, 33193, 45514, 41451, 53516, 49453, 61774, 57711, 4224, 161, 12482, 8419, 20484, 16421, 28742, 24679, 33721, 37784, 41979,
            46042, 49981, 54044, 58239, 62302, 689, 4752, 8947, 13010, 16949, 21012, 25207, 29270, 46570, 42443, 38312, 34185, 62830, 58703, 54572, 50445, 13538, 9411, 5280, 1153, 29798,
            25671, 21540, 17413, 42971, 47098, 34713, 38840, 59231, 63358, 50973, 55100, 9939, 14066, 1681, 5808, 26199, 30326, 17941, 22068, 55628, 51565, 63758, 59695, 39368,
            35305, 47498, 43435, 22596, 18533, 30726, 26663, 6336, 2273, 14466, 10403, 52093, 56156, 60223, 64286, 35833, 39896, 43963, 48026, 19061, 23124, 27191, 31254, 2801,
            6864, 10931, 14994, 64814, 60687, 56684, 52557, 48554, 44427, 40424, 36297, 31782, 27655, 23652, 19525, 15522, 11395, 7392, 3265, 61215, 65342, 53085, 57212, 44955,
            49082, 36825, 40952, 28183, 32310, 20053, 24180, 11923, 16050, 3793, 7920]

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.initUI()

    def initUI(self):
        global cal_bin

        self.setWindowTitle("Image Processing")
        self.setGeometry(100, 100, 50+50+256, 300)

        self.open_button = QPushButton("Open Image", self)
        self.open_button.setGeometry(20, 20, 100, 30)
        self.open_button.clicked.connect(self.open_image)

        self.process_button = QPushButton("Process Image", self)
        self.process_button.setGeometry(20, 70, 120, 30)
        self.process_button.clicked.connect(self.process_image)
        self.process_button.setEnabled(False)

        self.label = QLabel(self)
        self.label.setGeometry(50, 150, 256, 128)

        self.radioButton1 = QRadioButton("效果1", self)
        self.radioButton2 = QRadioButton("效果2", self)
        cal_bin = 1
        self.radioButton1.setGeometry(0, 100, 120, 30)
        self.radioButton2.setGeometry(50, 100, 120, 30)

        self.radioButton1.setChecked(True)  # 默认选中第一个单选按钮

        self.radioButton1.toggled.connect(self.on_radio_button_toggled)
        self.radioButton2.toggled.connect(self.on_radio_button_toggled)

        self.combo_box = QComboBox(self)
        self.combo_box.setGeometry(20, 120, 200, 30)
        self.populate_serial_ports()  # Populate available serial ports
        self.combo_box.currentIndexChanged.connect(self.on_combo_box_changed)

        self.timer = QTimer()
        self.timer.timeout.connect(self.populate_serial_ports)
        self.timer.start(500)  # Refresh every 5 seconds (1000 milliseconds)

        self.progress_bar = QProgressBar(self)
        self.progress_bar.setGeometry(50, 150, 200, 20)
        self.progress_bar.setValue(0)
    # ... (previous code remains the same)

    from PyQt5.QtWidgets import QComboBox

    class MainWindow(QMainWindow):
        def __init__(self):
            super().__init__()

            self.initUI()

        def initUI(self):
            # ... (existing code)

            self.combo_box = QComboBox(self)
            self.combo_box.setGeometry(20, 120, 200, 30)
            self.populate_serial_ports()  # Populate available serial ports
            self.combo_box.currentIndexChanged.connect(self.on_combo_box_changed)

    def populate_serial_ports(self):
        com_open = self.combo_box.currentText()
        import serial.tools.list_ports
        self.combo_box.clear()
        ports = serial.tools.list_ports.comports()
        for port in ports:
            self.combo_box.addItem(port.device)
        if com_open!="":
            index = self.combo_box.findText(com_open)
            if index >= 0:
                self.combo_box.setCurrentIndex(index)
            else:
                com_open = ""

    def qimage_to_gray_list(self,img):
        if img.isNull():
            return None

        width = img.width()
        height = img.height()

        pixel_list = []
        for y in range(height):
            for x in range(width):
                gray_value = QColor(img.pixelColor(x, y)).lightness()
                pixel_list.append(gray_value)

        return pixel_list

    def on_combo_box_changed(self, index):
        global com_open
        com_open = self.combo_box.currentText()

        # Debugging: print selected serial port
        print(f"Selected port: {com_open}")
    def on_radio_button_toggled(self):
        global cal_bin
        global resized_image
        global pixel_list

        sender = self.sender()

        if sender.isChecked():
            if sender == self.radioButton1:
                self.radioButton2.setChecked(False)
                if self.process_button.isEnabled() and cal_bin == 2:
                    binarized_image = self.binarize_image1(resized_image)
                    pixel_list=self.qimage_to_gray_list(binarized_image)
                    self.show_img(binarized_image)
                cal_bin=1
            else:
                self.radioButton1.setChecked(False)
                if self.process_button.isEnabled() and cal_bin == 1:
                    binarized_image = self.binarize_image2(resized_image)
                    pixel_list=self.qimage_to_gray_list(binarized_image)
                    self.show_img(binarized_image)
                cal_bin=2
    def open_image(self):
        global resized_image
        global cal_bin
        global pixel_list

        options = QFileDialog.Options()
        file_path, _ = QFileDialog.getOpenFileName(self, "Open Image File", "", "Image Files (*.jpg *.png *.bmp *.jpeg)",
                                                   options=options)
        if file_path:
            self.image_path = file_path
            self.process_button.setEnabled(True)
            try:
                original_image = QImage(self.image_path)
                resized_image = self.resize_image_qimage(original_image, 128, 64)
                if cal_bin==1:
                    binarized_image = self.binarize_image1(resized_image)
                else:
                    binarized_image = self.binarize_image2(resized_image)

                #binarized_image.save("C:/Users/RUPC/Desktop/3.jpg")
                pixel_list=self.qimage_to_gray_list(binarized_image)
                self.show_img(binarized_image)


                # 设置 label 为主窗口的中央部件
                # self.setCentralWidget(label)
            except Exception as e:
                print("Exception occurred:", str(e))

    def process_image(self):
        global pixel_list
        global com_open

        self.disable_all_widgets()
        if self.time_set()==False:
            self.enable_all_widgets()
            self.progress_bar.setValue(0)

            return False
        add=0x1E320
        num=128
        self.progress_bar.setValue(20)

        try:
            with serial.Serial(com_open, 38400, timeout=1) as ser:
                pass
                for i in range(8):
                    add1=add-0x10000
                    payload = b'\x38\x05' + b'\x8A\x00' + b'\x01\x00' + b'\x82\x00' + b'\x82\x40\x74\x65' +  add1.to_bytes(2, byteorder='little')
                    for value in pixel_list[i*128:i*128+128]:
                        payload += value.to_bytes(1, byteorder='big')  # 转换为字节并添加到 payload

                    # 将 payload 中的最后四个字节替换为当前时间戳
                    crc = self.crc16_ccitt(payload)
                    payload = payload + bytes([crc & 0xFF, ]) + bytes([crc >> 8, ])  # swap bytes of crc to get little endian

                    message = b'\xAB\xCD' + b'\x8e\x00' + self.payload_xor(payload) + b'\xDC\xBA'
                    hex_payload = ' '.join(hex(byte) for byte in message)
                    print(hex_payload)
                    ser.write(message)
                    full_response = ser.read(128)
                    if len(full_response) == 0:
                        self.message('写入失败')
                        self.progress_bar.setValue(0)
                        self.enable_all_widgets()
                        return False;
                    add=add+128
                    self.progress_bar.setValue(20+(i+1)*10)
            self.message('写入成功')
            self.progress_bar.setValue(0)
            self.enable_all_widgets()


        except serial.SerialException:
            self.self.message('写入失败')
            self.progress_bar.setValue(0)
            self.enable_all_widgets()
            return False;



    def disable_all_widgets(self):
        for widget in self.findChildren((QComboBox, QPushButton,QRadioButton)):
            widget.setEnabled(False)

    def enable_all_widgets(self):
        for widget in self.findChildren((QComboBox, QPushButton,QRadioButton)):
            widget.setEnabled(True)
    def show_img(self,binarized_image):
        pixmap = QPixmap.fromImage(self.resize_image_qimage(binarized_image, 256, 128))
        self.label.setPixmap(pixmap)
    def binarize_image1(self, original_image):
        binarized_image = original_image.convertToFormat(QImage.Format_Mono)
        return binarized_image

    def otsu_threshold(self,qimage):
        # Convert QImage to grayscale
        gray_qimage = qimage.convertToFormat(QImage.Format_Grayscale8)

        # Calculate histogram
        histogram = [0] * 256
        total_pixels = gray_qimage.width() * gray_qimage.height()

        for y in range(gray_qimage.height()):
            for x in range(gray_qimage.width()):
                pixel = QColor(gray_qimage.pixel(x, y))
                histogram[pixel.red()] += 1

        # Compute Otsu's threshold
        sum_total = 0
        for i in range(256):
            sum_total += i * histogram[i]

        sumB = 0
        wB = 0
        maximum = 0.0
        threshold = 0

        for i in range(256):
            wB += histogram[i]
            if wB == 0:
                continue
            wF = total_pixels - wB
            if wF == 0:
                break
            sumB += i * histogram[i]
            mB = sumB / wB
            mF = (sum_total - sumB) / wF
            between = wB * wF * (mB - mF) * (mB - mF)
            if between >= maximum:
                threshold = i
                maximum = between

        return threshold
    def binarize_image2(self, qimage):
        threshold = self.otsu_threshold(qimage)

        # Convert QImage to grayscale
        gray_qimage = qimage.convertToFormat(QImage.Format_Grayscale8)

        # Apply Otsu's thresholding
        for y in range(gray_qimage.height()):
            for x in range(gray_qimage.width()):
                pixel = QColor(gray_qimage.pixel(x, y))
                if pixel.red() > threshold:
                    gray_qimage.setPixel(x, y, QColor(255, 255, 255).rgb())  # White pixel
                else:
                    gray_qimage.setPixel(x, y, QColor(0, 0, 0).rgb())  # Black pixel

        return gray_qimage

    def resize_image_qimage(self, image,width,high):


        # 缩放图像
        resized_image = image.scaled(width, high)

        return resized_image

    def payload_xor(self,payload):
        XOR_ARRAY = bytes.fromhex('166c14e62e910d402135d5401303e980')
        XOR_LEN = len(XOR_ARRAY)

        ba = bytearray(payload)
        for i in range(0, len(ba)):
            ba[i] ^= XOR_ARRAY[i % XOR_LEN]
        return bytes(ba)

    def crc16_ccitt(self,data):
        i2 = 0
        for i3 in range(0, len(data)):
            out = Crc16Tab[((i2 >> 8) ^ data[i3]) & 255]
            i2 = out ^ (i2 << 8)

        return 65535 & i2

    def convert_payload_to_hex(self,code):
        payload_decoded = self.payload_xor(code[4:-4])  # 跳过头部和尾部的校验信息
        hex_payload = ' '.join(['{:02X}'.format(byte) for byte in payload_decoded])
        return hex_payload
    def message(self,text):
        msg_box = QMessageBox()
        msg_box.setWindowTitle("提示")
        msg_box.setText(text)
        msg_box.setIcon(QMessageBox.Critical)
        msg_box.setStandardButtons(QMessageBox.Ok)
        msg_box.exec()
    def time_set(self):
        global com_open
        try:
            with serial.Serial(com_open, 38400, timeout=1) as ser:
                settime = b'\xAB\xCD\x08\x00\x02\x69\x10\xE6\xAC\xD1\x79\x25\x9D\xAD\xDC\xBA'
                ser.write(settime)
                full_response = ser.read(128)
                if len(full_response) == 0:
                    self.populate_serial_ports()  # Populate available serial ports
                    self.message("连接失败")
                    return False  # Connection failed due to exception

        except serial.SerialException:
            self.message("连接失败")

            return False  # Connection failed due to exception

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.show()
    sys.exit(app.exec_())
