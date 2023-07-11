
import paho.mqtt.client as mqtt
import csv
import firebase_admin
from firebase_admin import credentials
from firebase_admin import firestore
import pandas as pd

#MQTT
broker = 'mqtt.flespi.io'
port = 1883
username = 'moJruoiv8YC1nbgzW2ffeFNi4cnCwNyIZ7Xjj6ChS2834esO7yU91xhVxvnT8JpV'
password = ''
topic = 'topic1'
topic2 = 'topic2'

uuid = None
pre_uuid = None
i = 1
client = mqtt.Client()

#Cloud
cred = credentials.Certificate('firebase-key.json')
firebase_admin.initialize_app(cred)
db = firestore.client()

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print("Failed to connect, return code %d\n", rc)

def on_message(client, userdata, msg):
    global uuid
    global i 
    payload = msg.payload.decode('utf-8')
    print(f'UUID: {payload}')
    #lấy uuid từ MQTT về
    uuid = payload #UUID từ MQTT

    collection_name = f'Lần gửi xe thứ {i}'  # Tạo tên bộ sưu tập mới dựa trên UUID
    document_name = 'Thông tin gửi xe'
    SaveToCSV(collection_name, document_name, uuid)

def SaveToCSV(collection_name, document_name, uuid):
    global pre_uuid
    global i 
    updated_rows = [] #dùng để so sánh với dữ liệu từ database
    with open('database.csv', 'r') as csv_file:
        csv_reader = csv.reader(csv_file)
        header = next(csv_reader)  # Lấy dòng tiêu đề

        for row in csv_reader:
            uuid_database = row[0]
            tinh_trang_database = int(row[1])
            xe_chungcu_database = int(row[2])
            if (uuid == uuid_database and uuid != pre_uuid):              
                print(uuid)
                pre_uuid = uuid
                tinh_trang_database = not tinh_trang_database
                if tinh_trang_database == True:
                    tinh_trang_database = 1
                else:
                    tinh_trang_database = 0
                client.publish(topic2, tinh_trang_database)
                i = i + 1

                updated_row = [uuid_database, tinh_trang_database, xe_chungcu_database]
                updated_rows.append(updated_row)
                for data in updated_rows:
                    data = {
                        'uuid': uuid_database,
                        'tinh_trang': tinh_trang_database,
                        'xecc': xe_chungcu_database
                    }
                doc_ref = db.collection(collection_name).document(document_name)
                doc_ref.set(data)
            else:
                updated_rows.append(row)

    with open('database.csv', 'w', newline='') as file:
        csv_writer = csv.writer(file)
        csv_writer.writerow(header) 
        csv_writer.writerows(updated_rows) 
    
def connect_mqtt() -> mqtt:
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client

def subscribe(client: mqtt):
    client.subscribe(topic)
    client.on_message = on_message

def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()

if __name__ == '__main__':
    run()
