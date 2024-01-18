# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'd:\Major Courses\DataBase\lab\LAB2\LAB2-code\intlflight.ui'
#
# Created by: PyQt5 UI code generator 5.15.4
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import QMessageBox
from ui_table import table
import pymysql
import pandas as pd
class GoldFlight(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(1500, 600)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.textEdit = QtWidgets.QTextEdit(self.centralwidget)
        self.textEdit.setGeometry(QtCore.QRect(270, 100, 800, 61))
        self.textEdit.setObjectName("textEdit")
        self.label = QtWidgets.QLabel(self.centralwidget)
        self.label.setGeometry(QtCore.QRect(210, 240, 100, 30))
        self.label.setObjectName("label")
        self.lineEdit = QtWidgets.QLineEdit(self.centralwidget)
        self.lineEdit.setGeometry(QtCore.QRect(320, 240, 161, 20))
        self.lineEdit.setObjectName("lineEdit")
        self.commandLinkButton = QtWidgets.QCommandLinkButton(self.centralwidget)
        self.commandLinkButton.setGeometry(QtCore.QRect(370, 350, 186, 41))
        self.commandLinkButton.setObjectName("commandLinkButton")
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 800, 22))
        self.menubar.setObjectName("menubar")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)
        self.commandLinkButton.clicked.connect(self.query)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "MainWindow"))
        self.textEdit.setHtml(_translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:\'SimSun\'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:28pt; font-weight:600;\">请输入机场</span></p></body></html>"))
        self.label.setText(_translate("MainWindow", "机场名"))
        self.commandLinkButton.setText(_translate("MainWindow", "确认"))
    #嵌套查询&连接查询
    def query(self):
        airport=self.lineEdit.text()
        if airport=="":
            QMessageBox.warning(self.window,"ERROR!","输入不能为空")
        else:
            con = pymysql.connect(host='localhost', port=3306, user='root', password='000000', charset='utf8',database='airlines')  # 连接数据库
            sql1=f"select flight.fno,S.aname as sa,ttime,T.aname as ta,ltime,state,model from takeoff,landon,airport as S,airport as T,plane,flight,execute \
                where takeoff.fno=landon.fno and takeoff.aname=S.aname and landon.aname=T.aname and flight.fno=takeoff.fno and flight.fno=landon.fno and flight.fno=execute.fno and execute.planeid=plane.planeid"
            sql=f"select * from ({sql1}) as FlightInfo where (sa='{airport}' and CONVERT(ttime,SIGNED)>1000 and CONVERT(ttime,SIGNED)<1800) or (ta='{airport}' and CONVERT(ltime,SIGNED)<1800 and CONVERT(ltime,SIGNED)>1000)"
            df=pd.read_sql(sql,con)
            self.form=QtWidgets.QWidget()
            self.table=table()
            self.table.setupUi(self.form,len(df),len(df.columns),[x for x in range(1,len(df)+1)],["航班号","起飞机场","起飞时间","降落机场","降落时间","状态","机型"])
            for i in range(len(df)):
                for j in range(len(df.columns)):
                    self.table.tableWidget.setItem(i, j, QtWidgets.QTableWidgetItem(str(df.iat[i,j]).strip()))
            self.form.show()
            con.close()
        
