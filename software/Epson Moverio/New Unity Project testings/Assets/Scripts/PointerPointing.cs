using UnityEngine;
using System.Collections;

public class PointerPointing : MonoBehaviour {

    public Transform target;

	// Update is called once per frame
	void Update () {
        Vector3 relativePos = target.position - transform.position;
        Quaternion rotation = Quaternion.LookRotation(relativePos);
        transform.rotation = rotation;
	}
}
