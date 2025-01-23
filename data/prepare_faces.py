import os, sys
import face_recognition
import json
import re

face_db_path = "faces/"

persons = []

for img_path in os.listdir(face_db_path):

    parts = img_path[:-4].split(":")
    print(parts)
    person_id = parts[0]
    name = parts[1]
    img = face_recognition.load_image_file(face_db_path + img_path)
    encoding = face_recognition.face_encodings(img)[0]

    encoding_file_name = "face_" + person_id + ".json"

    encoding_content = json.dumps(encoding.tolist())

    with open(encoding_file_name, "w") as f:
        f.write(encoding_content)

    person = {
        "id" : int(person_id),
        "name" : name,
        "name_known" : True,
        "encoding_file" : encoding_file_name
    }

    persons.append(person)

output = json.dumps(persons, indent=4)

with open("people.json", "w") as f:
    f.write(output)

