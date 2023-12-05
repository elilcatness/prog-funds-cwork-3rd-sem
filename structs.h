#include <iostream>

using namespace std;

// Транзакция
#pragma once
struct Order {
	size_t id; // идентификатор транзакции
	uint32_t sender; // номер счёта отправителя
	uint32_t recipient; // номер счёта получателя
	float amount; // сумма транзакции
	Order* nextSender = nullptr; // указатель на следующую транзакцию с иным отправителем,
	// т.е. переход к следующей группе транзакций
	Order* next = nullptr;
};

// Счёт
#pragma once
struct Account {
	uint32_t id; // номер счёта
	string password; // пароль от счёта
	float balance = 0.0f; // баланс на счёте
	Account* next = nullptr; // указатель на следующий счёт
};