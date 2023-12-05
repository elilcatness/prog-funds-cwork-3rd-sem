#include <iostream>

using namespace std;

// ����������
#pragma once
struct Order {
	size_t id; // ������������� ����������
	uint32_t sender; // ����� ����� �����������
	uint32_t recipient; // ����� ����� ����������
	float amount; // ����� ����������
	Order* nextSender = nullptr; // ��������� �� ��������� ���������� � ���� ������������,
	// �.�. ������� � ��������� ������ ����������
	Order* next = nullptr;
};

// ����
#pragma once
struct Account {
	uint32_t id; // ����� �����
	string password; // ������ �� �����
	float balance = 0.0f; // ������ �� �����
	Account* next = nullptr; // ��������� �� ��������� ����
};