using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class SensorInformation : MonoBehaviour {

    public Text Compasstxt;
    public Text Gyroscopetxt;
    public Text Accelerotxt;

    private Compass myCompass;
    private Gyroscope myGyroscope;
    private string compasstext;
    private string gyroscopetext;
    private string accelerotext;

	// Use this for initialization
	void Start () {
        myCompass = Input.compass;
        myGyroscope = Input.gyro;
        myCompass.enabled = true;
        myGyroscope.enabled = true;
	}
	
	// Update is called once per frame
	void FixedUpdate () {
        compasstext = "Compass \nenabled: " + myCompass.enabled + "\nheadingAccuracy: \n" + myCompass.headingAccuracy + "\nmagneticHeading: \n" + myCompass.magneticHeading + "\nrawVector xyz: \n" + myCompass.rawVector.x + "\n" + myCompass.rawVector.y + "\n" + myCompass.rawVector.z + "\ntimestamp: \n" + myCompass.timestamp + "\ntrueHeading: \n" + myCompass.trueHeading;
        gyroscopetext = "Gyroscope \nattitude xyz: \n" + myGyroscope.attitude.x + "\n" + myGyroscope.attitude.y + "\n" + myGyroscope.attitude.z + "\nenabled: \n" + myGyroscope.enabled + "\ngravity xyz: \n" + myGyroscope.gravity.x + "\n" + myGyroscope.gravity.y + "\n" + myGyroscope.gravity.z + "\nrotationRate xyz: \n" + myGyroscope.rotationRate.x + "\n" + myGyroscope.rotationRate.y + "\n" + myGyroscope.rotationRate.z + "\nrotationRateUnbiased xyz: \n" + myGyroscope.rotationRateUnbiased.x + "\n" + myGyroscope.rotationRateUnbiased.y + "\n" + myGyroscope.rotationRateUnbiased.z + "\nupdateInterval: \n" + myGyroscope.updateInterval + "\nuserAcceleration: \n" + myGyroscope.userAcceleration;
        accelerotext = "Accelero \nmagnitude: \n" + Input.acceleration.magnitude + "\nnormalized xyz:\n " + Input.acceleration.normalized.x + "\n" + Input.acceleration.normalized.y + "\n" + Input.acceleration.normalized.z + "\nsqrMagnitude: \n" + Input.acceleration.sqrMagnitude + "\nraw xyz: \n" + Input.acceleration.x + "\n" + Input.acceleration.y + "\n" + Input.acceleration.z;

        Compasstxt.text = compasstext;
        Gyroscopetxt.text = gyroscopetext;
        Accelerotxt.text = accelerotext;

        //Compasstxt.text = "Jip";
        //Gyroscopetxt.text = "is";
        //Accelerotxt.text = "banaan";
	}
}
