#!/usr/bin/env python3
"""
MAZDUINO WebServer Connection Test
Tests webserver accessibility on different IP addresses
"""

import requests
import socket
import time
import sys
from urllib.parse import urljoin

def test_connection(ip_address, timeout=5):
    """Test HTTP connection to specific IP address"""
    url = f"http://{ip_address}/"
    
    try:
        print(f"Testing connection to {url}...")
        response = requests.get(url, timeout=timeout)
        
        if response.status_code == 200:
            print(f"✓ SUCCESS: WebServer accessible at {url}")
            print(f"  Response time: {response.elapsed.total_seconds():.3f}s")
            print(f"  Content length: {len(response.text)} bytes")
            # Check if it's the MAZDUINO dashboard
            if "MAZDUINO" in response.text or "Dashboard" in response.text:
                print(f"  ✓ MAZDUINO Dashboard detected")
            return True
        else:
            print(f"✗ HTTP Error: Status {response.status_code}")
            return False
            
    except requests.exceptions.ConnectTimeout:
        print(f"✗ CONNECTION TIMEOUT: No response from {url} after {timeout}s")
        return False
    except requests.exceptions.ConnectionError as e:
        print(f"✗ CONNECTION ERROR: {e}")
        return False
    except Exception as e:
        print(f"✗ UNEXPECTED ERROR: {e}")
        return False

def test_socket_connection(ip_address, port=80, timeout=3):
    """Test raw socket connection"""
    try:
        print(f"Testing socket connection to {ip_address}:{port}...")
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(timeout)
        result = sock.connect_ex((ip_address, port))
        sock.close()
        
        if result == 0:
            print(f"✓ Socket connection successful to {ip_address}:{port}")
            return True
        else:
            print(f"✗ Socket connection failed to {ip_address}:{port} (error: {result})")
            return False
    except Exception as e:
        print(f"✗ Socket test error: {e}")
        return False

def main():
    print("=== MAZDUINO WebServer Connection Test ===")
    print("This script tests webserver accessibility on different interfaces")
    print()
    
    # Default IP addresses to test
    ap_ip = "192.168.4.1"      # Default AP mode IP
    sta_ip = "10.1.1.28"       # User reported STA IP
    
    # Allow custom IP from command line
    if len(sys.argv) > 1:
        sta_ip = sys.argv[1]
        print(f"Using custom STA IP: {sta_ip}")
    
    test_results = {}
    
    print("Phase 1: Socket Connection Tests")
    print("-" * 40)
    test_results['ap_socket'] = test_socket_connection(ap_ip)
    test_results['sta_socket'] = test_socket_connection(sta_ip)
    print()
    
    print("Phase 2: HTTP Connection Tests")
    print("-" * 40)
    test_results['ap_http'] = test_connection(ap_ip)
    test_results['sta_http'] = test_connection(sta_ip)
    print()
    
    print("=== SUMMARY ===")
    print(f"AP IP ({ap_ip}):")
    print(f"  Socket: {'✓ PASS' if test_results['ap_socket'] else '✗ FAIL'}")
    print(f"  HTTP:   {'✓ PASS' if test_results['ap_http'] else '✗ FAIL'}")
    
    print(f"STA IP ({sta_ip}):")
    print(f"  Socket: {'✓ PASS' if test_results['sta_socket'] else '✗ FAIL'}")
    print(f"  HTTP:   {'✓ PASS' if test_results['sta_http'] else '✗ FAIL'}")
    print()
    
    # Diagnosis
    if test_results['ap_http'] and not test_results['sta_http']:
        print("DIAGNOSIS: WebServer only accessible via AP interface")
        print("POSSIBLE CAUSES:")
        print("- WebServer not binding to STA interface")
        print("- Firewall blocking STA interface access")
        print("- Router network isolation")
        print("- ESP32 routing table issues")
        print()
        print("RECOMMENDATIONS:")
        print("1. Check ESP32 Serial Monitor for IP binding info")
        print("2. Try ping test to STA IP")
        print("3. Check if device responds to ping but not HTTP")
        print("4. Verify router firewall settings")
        
    elif test_results['sta_socket'] and not test_results['sta_http']:
        print("DIAGNOSIS: STA IP reachable but HTTP service not responding")
        print("POSSIBLE CAUSES:")
        print("- WebServer process crashed/restarted")
        print("- HTTP service only listening on AP interface")
        print("- Port 80 blocked by ESP32 firewall")
        
    elif not test_results['sta_socket']:
        print("DIAGNOSIS: STA IP not reachable at network level")
        print("POSSIBLE CAUSES:")
        print("- Device not actually connected to router")
        print("- IP address changed/expired")
        print("- Network routing issues")
        print("- Device network interface down")

if __name__ == "__main__":
    main()