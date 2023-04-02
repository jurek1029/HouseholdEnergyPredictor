import requests
lat = -33.856784
long = 151.215297
cap=5
tilt=42
azimuth=180
hours=168
# endpoint = f"https://api.solcast.com.au/data/forecast/rooftop_pv_power?latitude={lat}&longitude={long}&output_parameters=pv_power_rooftop&capacity={cap}&tilt={tilt}&azimuth={azimuth}&hours={hours}&format=json"
# print(endpoint)

# headers = {"Authorization": "Bearer q_m_SaOSVkHui2bOMrGgzY6T68TbEpN7",
# "Accept": "application/json"}
# res = requests.post(endpoint, headers=headers)
# print(res)
# print(res.json())


#radiation
url = "https://api.solcast.com.au/world_radiation/forecasts?latitude=-33.856784&longitude=151.215297&output_parameters=air_temp,dni,ghi"
#PVPower
url = f"https://api.solcast.com.au/world_pv_power/forecasts?latitude={lat}&longitude={long}&output_parameters=pv_power_rooftop&capacity={cap}&tilt={tilt}&azimuth={azimuth}&hours={hours}&format=csv"



payload={}
headers = {
  'Accept': 'application/json',
  "Authorization": "Bearer q_m_SaOSVkHui2bOMrGgzY6T68TbEpN7"
}

response = requests.request("GET", url, headers=headers, data=payload)

print(response.text)
print(len(response.text))
print("Url Len:", len(url))
print(url)