#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <crtdbg.h>

#include <fstream>
#include <string>
#include <math.h>
#include <iomanip>

#include "structs.h"
#include "dt.h"
#include "exceptions.h"
#include "utils.h"
#include "conio.h"


// TODO: округлять баланс до двух знаков после запятой, проверка на +
Account* loadAccounts(const string filename = "accounts.txt") {
	checkFileExtension(filename);
	return new Account{ 123, "123" };
	//return nullptr;
}

// TODO: округлять сумму платежа до двух знаков после запятой, проверка на +
Order* loadOrders(const string filename = "orders.txt") {
	checkFileExtension(filename);
	ifstream f(filename);
	//if (!f.is_open()) {
	//	return nullptr;
	//	/*throw FileNotFound("Файл " + filename + " не найден");*/
	//}
	f.close();
	return nullptr;
	/*if (f.eof())
		return nullptr;
	f.close();*/
	/*Order* head = nullptr;
	size_t ordersCount;*/
}

void showAccounts(Account* ptr) {
	while (ptr != nullptr) {
		showHeader("Счёт #" + to_string(ptr->id));
		cout << "Баланс: " << ptr->balance << "\n\n";
		ptr = ptr->next;
	}
}

Account* authorize(Account* accounts) {
	Account* cur;
	try {
		while ((cur = findAccount(accounts, getVar<uint32_t>(
			"Введите номер счёта"))) == nullptr)
			cout << "Введён несуществующий номер счёта!" << endl;
	}
	catch (EmptyInputException&) {
		return nullptr;
	}
	/*while (true) {
		try {
			id = getVar<uint32_t>("Введите номер счёта");
		}
		catch (EmptyInputException& e) {
			return nullptr;
		}
		if ((cur = findAccount(accounts, id)) == nullptr)
			cout << "Введён несуществующий номер счёта!" << endl;
		else break;
	}*/
	try {
		while (getVar<string>("Введите пароль") != cur->password)
			cout << "Неверный пароль" << endl;
	}
	catch (EmptyInputException&) {
		return nullptr;
	}
	return cur;
}

void add(Account* accounts, Order*& orders, Account*& curAccount,
		uint32_t& maxAccountId, size_t& maxOrderId) {
	string password;
	uint32_t recipientId;
	Account* recipient = nullptr;
	Order* order = nullptr;
	float amount;
	bool isRunning = true;
	while (isRunning) {
		system("cls");
		showHeader("Добавление / перевод");
		cout << curAccount;
		cout << "1. Создать новый счёт" << endl;
		if (curAccount != nullptr)
			cout << "2. Совершить транзакцию" << endl;
		cout << "0. Вернуться назад" << endl;
		switch (_getch()) {
		case '1':
			system("cls");
			showHeader("Добавление счёта");
			if (maxAccountId < numeric_limits<uint32_t>::max() - 1) {
				try {
					password = getVar<string>("Введите пароль");
				}
				catch (EmptyInputException&) {
					break;
				}
				curAccount = new Account{ ++maxAccountId, password };
				linkedAppend(accounts, curAccount);
				cout << "Счёт #" << maxAccountId << " был успешно добавлен!" << endl;
			} 
			else
				cout << "Достигнуто максимальное количество аккаунтов" << endl;
			pause();
			break;
		case '2':
			system("cls");
			showHeader("Меню перевода");
			cout << curAccount;
			if (curAccount == nullptr)
				break;
			if (maxOrderId < numeric_limits<size_t>::max() - 1) {
				try {
					while (true) {
						recipientId = getVar<uint32_t>("Введите номер счёта получателя");
						if (recipientId == curAccount->id)
							cout << "Вы не можете сделать перевод самому себе!" << endl;
						else if ((recipient = findAccount(accounts, recipientId)) == nullptr)
							cout << "Счёт с таким номером не существует!" << endl;
						else break;
					}
					while ((amount = floor(getVar<float>(
							"Введите сумму платежа") * 100) / 100) <= 0)
						cout << "Сумма должна быть больше нуля (с учётом округления)!" << endl;
				}
				catch (EmptyInputException&) {
					break;
				}
				curAccount->balance -= amount;
				recipient->balance += amount;
				appendOrder(orders, new Order{ ++maxOrderId, curAccount->id, recipientId, amount });
				cout << "Транзакция #" << maxOrderId << " была успешно совершена!" << endl;
			} 
			else
				cout << "Достигнуто максимальное количество транзакций" << endl;
			pause();
			break;
		case '0':
			isRunning = false;
			break;
		}
	}
}

void viewIncome(Account* accounts, Order* orders, Account* curAccount) {
	Order *cur, *start, *end;
	float income;
	uint32_t senderId;
	bool isRunning = true, anyOrder;
	while (isRunning) {
		system("cls");
		showHeader("Доход");
		cout << curAccount;
		cout << "1. Общий доход\n2. Доход по отправителю\n"
			<< "0. Вернуться назад" << endl;
		cur = orders, income = 0;
		switch (_getch()) {
		case '1':
			system("cls");
			showHeader("Общий доход");
			cout << curAccount << endl;
			while (cur != nullptr) {
				if (cur->recipient == curAccount->id)
					income += cur->amount;
				cur = cur->next;
			}
			cout << "Ваш общий доход: " << income << endl;
			pause();
			break;
		case '2':
			system("cls");
			showHeader("Доход по отправителю");
			cout << curAccount << endl;
			try {
				while (true) {
					senderId = getVar<uint32_t>("Введите номер счёта отправителя");
					if (senderId == curAccount->id)
						cout << "Вы не можете вводить свой номер своего счёта!" << endl;
					else if (findAccount(accounts, senderId) == nullptr)
						cout << "Счёта с таким номером не существует!" << endl;
					else break;
				}
			}
			catch (EmptyInputException&) {
				break;
			}
			if (anyOrder = iterSenderOrders(orders, senderId, start, end)) {
				anyOrder = false;
				while (start != end) {
					if (start->recipient == curAccount->id) {
						if (!anyOrder)
							anyOrder = true;
						income += start->amount;
					}
					start = start->next;
				}
			}
			if (anyOrder)
				cout << "Доход со счёта #" << senderId
				<< ": " << income << endl;
			else
				cout << "На Ваш счёт не поступало никаких средств "
					<< "со счёта #" << senderId << endl;
			pause();
			break;
		case '0':
			isRunning = false;
		}
	}
}

void view(Account* accounts, Order* orders, Account* curAccount) {
	Order *cur, *start, *end;
	bool isRunning[] = { true, false }, displayedAny;
	while (isRunning[0]) {
		system("cls");
		showHeader("Просмотр и подсчёт");
		cout << curAccount;
		cout << "1. Просмотр транзакций\n2. Доход / расход / прибыль"
			"\n3. Данные о счёте\n0. Вернуться назад" << endl;
		switch (_getch()) {
		case '1':
			isRunning[1] = true;
			while (isRunning[1]) {
				system("cls");
				showHeader("Просмотр транзакций");
				cout << curAccount;
				cout << "1. Входящие\n2. Исходящие\n0. Вернуться назад" << endl;
				switch (_getch()) {
				case '1':
					system("cls");
					showHeader("Входящие транзакции");
					cout << curAccount;
					cur = orders;
					displayedAny = false;
					while (cur != nullptr) {
						if (cur->recipient == curAccount->id) {
							if (!displayedAny)
								displayedAny = true;
							cout << cur << endl;
						}
						cur = cur->next;
					}
					if (!displayedAny)
						cout << "У Вас нет входящих транзакций" << endl;
					pause();
					break;
				case '2':
					system("cls");
					showHeader("Исходящие транзакции");
					cout << curAccount;
					//if (!showSenderOrders(orders, curAccount->id))
					if (iterSenderOrders(orders, curAccount->id, start, end))
						while (start != end) {
							cout << start << endl;
							start = start->next;
						}
					else
						cout << "У Вас нет исходящих транзакций" << endl;
					pause();
					break;
				case '0':
					isRunning[1] = false;
					break;
				}
			}
			break;
		case '2':
			isRunning[1] = true;
			while (isRunning[1]) {
				system("cls");
				showHeader("Доход / расход / прибыль");
				cout << curAccount;
				cout << "1. Доход\n2. Расход\n3. Прибыль\n0. Вернуться назад" << endl;
				switch (_getch()) {
				case '1':
					viewIncome(accounts, orders, curAccount);
					break;
				case '2':
					break;
				case '3':
					break;
				case '0':
					isRunning[1] = false;
					break;
				}
			}
			break;
		case '3':
			system("cls");
			showHeader("Данные о счёте");
			cout << curAccount;
			cout << "Баланс: " << fixed << setprecision(2) << curAccount->balance << endl;
			pause();
			break;
		case '0':
			isRunning[0] = false;
		}
	}
}

void process() {
	Account *accounts = nullptr, *curAccount = nullptr, *tmp;
	uint32_t maxAccountId = 0;
	try {
		accounts = loadAccounts("sa.env.txt");
	} 
	catch (exception& e) {
		cout << typeid(e).name() << ": " << e.what() << endl;
	}
	Order* orders = nullptr;
	size_t maxOrderId = 0;
	string menu[] = { "1. Авторизация", "2. Добавление / перевод", "3. Редактирование", 
					  "4. Удаление", "5. Просмотр и подсчёт", "6. Сортировка", 
					  "7. Сохранить БД", "8. Загрузить БД", "0. Выход" };
	const uint16_t menuSize = sizeof(menu) / sizeof(string);
	string* noAuthMenu[] = { &menu[0], &menu[1], &menu[menuSize - 1] };
	const uint16_t noAuthMenuSize = sizeof(noAuthMenu) / sizeof(string*);
	uint16_t i;
	bool isRunning = true;
	while (isRunning) {
		system("cls");
		showHeader("Меню");
		if (curAccount != nullptr) {
			cout << curAccount;
			for (i = 0; i < menuSize; i++)
				cout << menu[i] << endl;
		}
		else
			for (i = 0; i < noAuthMenuSize; i++)
				cout << *noAuthMenu[i] << endl;
		// TODO: вынести код из кейсов в функции и вызывать по индексу
		switch (_getch()) {
		case '1':
			system("cls");
			showHeader("Авторизация");
			if ((tmp = authorize(accounts)) != nullptr)
				curAccount = tmp;
			break;
		case '2':
			system("cls");
			add(accounts, orders, curAccount, maxAccountId, maxOrderId);
			break;
		case '3': {
			;
			break;
		}
		case '4':
			;
			break;
		case '5':
			if (curAccount == nullptr)
				break;
			view(accounts, orders, curAccount);
			break;
		case '6': {
			break;
		}
		case '7':
			;
			break;
		case '8':
			;
			break;
		case '9':
			;
			break;
		case '0':
			cout << "\nЗавершение работы программы..." << endl;
			isRunning = false;
			break;
		}
	}
	freeLinked(accounts);
	freeLinked(orders);
}

int main() {
	process();
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
	_CrtDumpMemoryLeaks();
}