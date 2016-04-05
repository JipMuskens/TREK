using UnityEngine;
using System.Collections;
using System.Linq;

public class Locations : MonoBehaviour {

	// Use this for initialization
    public UDPManager myUDPManager;
    public Camera myCamera;
    public GameObject mySphere;
    private Vector3 zeroVector;

	void Start () {
        zeroVector.x = 0;
        zeroVector.y = 0;
        zeroVector.z = 0;
	}
	
	// Update is called once per frame
	void Update () {
        if (myUDPManager != null)
        {
            Vector3 newCamVector = new Vector3((float)myUDPManager.tagdata.xGlass , 0, (float)myUDPManager.tagdata.yGlass ); // xglass, zglass, yglass
            myCamera.transform.position = newCamVector;

            Vector3 newSphereVector = new Vector3((float)myUDPManager.tagdata.xTag , 0, (float)myUDPManager.tagdata.yTag ); // xtag, ztag, ytag

            mySphere.transform.position = newSphereVector; // Literally teleport to the location that is received over Wi-Fi
            //mySphere.transform.position = Vector3.MoveTowards(transform.position, newSphereVector, 10 * Time.deltaTime); // Slowly move towards the location that is received over Wi-Fi
        }
	}
}
