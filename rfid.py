import serial
import mysql.connector

arduino = serial.Serial('COM3', 9600)

db = mysql.connector.connect(
    host="localhost",
    user="root",
    password="",
    database="Attendance"
)

cursor = db.cursor()

print("Waiting for RFID data...")

while True:
    if arduino.in_waiting > 0:
        line = arduino.readline().decode('utf-8').strip()

        if line.startswith("DATA"):
            try:
                parts = line.split(",")

                name = parts[1]
                uid = parts[2]
                datetime = parts[3]

                # Split into DATE and TIME
                date_part = datetime.split(" ")[0]
                time_part = datetime.split(" ")[1]

                sql = """
                INSERT INTO logs (Student_name, card_uid, Date, Time)
                VALUES (%s, %s, %s, %s)
                """

                values = (name, uid, date_part, time_part)

                cursor.execute(sql, values)
                db.commit()

                print("Saved:", name, uid, date_part, time_part)

            except Exception as e:
                print("Error:", e)