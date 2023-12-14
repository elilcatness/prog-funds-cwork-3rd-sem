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

// TODO: больше почитать про функции
// TODO: сделать отдельный файл для максимальных ID: account & order


Account* loadAccounts(uint32_t& maxId, const string filename = "accounts.txt") {
	checkFileExtension(filename);
	ifstream f(filename);
	if (!f.is_open())
		return nullptr;
	uint32_t count, id;
	float balance;
	string password;
	count = getVarFromFile<uint32_t>(f);
	Account *accounts = nullptr, *cur = nullptr, *prev = nullptr;
	for (uint32_t i = 0; i < count; i++) {
		id = getVarFromFile<uint32_t>(f);
		password = getVarFromFile<string>(f);
		balance = getVarFromFile<float>(f, '\n', i == count - 1);
		cur = new Account{ id, password, balance != 0 ? floor(balance * 100) / 100 : 0};
		if (id > maxId)
			maxId = id;
		if (accounts == nullptr)
			accounts = cur;
		else if (prev != nullptr)
			prev->next = cur;
		prev = cur;
	}
	f.close();
	cout << "Загружено счетов: " << count << endl;
	pause();
	return accounts;
}

bool dumpAccounts(Account* accounts, 
		const string filename = "accounts.txt") {
	if (accounts == nullptr)
		return false;
	checkFileExtension(filename);
	ofstream f(filename);
	if (!f.is_open())
		return false;
	f << endl;
	uint32_t count = 0;
	while (accounts != nullptr) {
		f << accounts->id << endl << accounts->password << endl
			<< fixed << setprecision(2) << accounts->balance << endl;
		accounts = accounts->next;
		count++;
	}
	f.seekp(0);
	f << count;
	f.close();
	return true;
}

Order* loadOrders(size_t& maxId, const string filename = "orders.txt") {
	checkFileExtension(filename);
	ifstream f(filename);
	if (!f.is_open())
		return nullptr;
	Order *orders = nullptr, *cur = nullptr, *prevSender = nullptr, *prev = nullptr;
	Order* senderHead = nullptr;
	uint32_t senderId, senderCount, recipientId;
	size_t orderCount, orderId;
	float amount;
	senderCount = getVarFromFile<uint32_t>(f);
	bool isLastSender;
	size_t count = 0;
	for (uint32_t i = 0; i < senderCount; i++) {
		senderId = getVarFromFile<uint32_t>(f, ' ');
		orderCount = getVarFromFile<size_t>(f);
		isLastSender = i == senderCount - 1;
		//f >> senderId >> orderCount;
		for (size_t j = 0; j < orderCount; j++) {
			orderId = getVarFromFile<size_t>(f, ' ');
			recipientId = getVarFromFile<uint32_t>(f, ' ');
			amount = getVarFromFile<float>(
				f, '\n', isLastSender && j == orderCount - 1);
			if (amount <= 0)
				throw InvalidFileFormat(
					"Некорректный формат файла: суммы переводов должны быть положительны");
			//f >> orderId >> recipientId >> amount;
			if (orderId > maxId)
				maxId = orderId;
			cur = new Order{ orderId, senderId, recipientId, amount };
			if (orders == nullptr)
				orders = cur;
			if (j == 0) {
				senderHead = cur;
				if (prevSender != nullptr)
					prevSender->nextSender = cur;
			}
			if (prev != nullptr)
				prev->next = cur;
			prev = cur;
			count++;
		}
		prevSender = senderHead;
	}
	cout << "\nЗагружено транзакций: " << count << endl;
	pause();
	f.close();
	return orders;
}

bool dumpOrders(Order* orders,
		const string filename = "orders.txt") {
	if (orders == nullptr)
		return false;
	checkFileExtension(filename);
	ofstream f(filename);
	if (!f.is_open())
		return false;
	f << endl;
	uint32_t senderCount = 0, prevSenderId = 0;
	size_t p = 0, tmpP, orderCount = 0;
	while (orders != nullptr) {
		if (orders->sender != prevSenderId) {
			prevSenderId = orders->sender;
			if (p) {
				tmpP = f.tellp();
				f.seekp(p);
				f << orderCount;
				f.seekp(tmpP);
				orderCount = 0;
			}
			f << orders->sender << ' ';
			p = f.tellp();
			f << endl;
			senderCount++;
		}
		f << orders->id << ' ' << orders->recipient << ' '
			<< fixed << setprecision(2) << orders->amount << endl;
		orders = orders->next;
		orderCount++;
	}
	if (p) {
		f.seekp(p);
		f << orderCount;
	}
	f.seekp(0);
	f << senderCount;
	f.close();
	return true;
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

void add(Account*& accounts, Order*& orders, Account*& curAccount,
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
			if (curAccount == nullptr)
				break;
			cout << curAccount;
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
					amount = getAmount();
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

void edit(Account* accounts, Order* orders, Account* curAccount) {
	Order* order = nullptr, * start, * end;
	size_t orderId;
	float amount, diff;
	bool abort = false;
	while (true) {
		system("cls");
		showHeader("Редактирование исходящих транзакций");
		cout << curAccount;
		if (!iterSenderOrders(orders, curAccount->id, start, end)) {
			cout << "У Вас нет исходящих транзакций!" << endl;
			pause();
			return;
		}
		try {
			while ((orderId = getVar<size_t>("Введите номер транзакции")) == 0) {
				cout << "Некорректный ввод!" << endl;
			}
		}
		catch (EmptyInputException&) {
			return;
		}
		order = start;
		while (order != end) {
			if (order->id == orderId) {
				cout << order << endl;
				try {
					amount = getAmount();
				}
				catch (EmptyInputException&) {
					abort = true;
					break;
				}
				diff = order->amount - amount;
				curAccount->balance += diff;
				findAccount(accounts, order->recipient)->balance -= diff;
				cout << "Сумма транзакции #" << order->id
					<< " была изменена с " << order->amount
					<< " на " << amount << endl;
				order->amount = amount;
				pause();
				return;
			}
			order = order->next;
		}
		if (!abort)
			cout << "Транзакции #" << orderId << " не существует!" << endl;
		else
			abort = false;
	}
}

void remove(Account*& accounts, Order*& orders, Account*& curAccount) {
	Order *cur, *prev, *prevSender;
	Account* recipient;
	size_t orderId;
	bool found;
	while (true) {
		system("cls");
		showHeader("Удаление");
		cout << curAccount << "1. Отменить транзакцию\n2. Аннулировать счёт\n"
							  "0. Вернуться назад" << endl;
		switch (_getch()) {
		case '1':
			found = false;
			system("cls");
			showHeader("Отмена транзакции");
			cout << curAccount;
			cur = orders;
			prevSender = getPrevSender(orders, curAccount->id, cur);
			/*prevSender = nullptr, cur = orders;
			while (cur != nullptr && cur->sender != curAccount->id) {
				prevSender = cur;
				cur = cur->nextSender;
			}*/
			if (cur == nullptr) {
				cout << "У Вас нет исходящих транзакций!" << endl;
				pause();
				break;
			}
			try {
				while ((orderId = getVar<size_t>("Введите номер транзакции")) == 0)
					cout << "Некорректный ввод!" << endl;
			}
			// TODO: посмотреть в отладке, как работает вызов и ловля исключения
			catch (EmptyInputException&) {
				continue;
			}
			// || cur == nullptr добавлено, чтобы не лезли ворнинги
			if (!assignPrev(orders, curAccount->id, orderId, prevSender, cur, prev) ||
					cur == nullptr) {
				cout << "Транзакция #" << orderId << " не найдена!" << endl;
				pause();
				break;
			}
			/*prev = nullptr;
			while (cur != nullptr && cur->sender == curAccount->id && 
					cur->id != orderId) {
				prev = cur;
				cur = cur->next;
			}
			if (cur == nullptr || cur->sender != curAccount->id) {
				cout << "Транзакция #" << orderId << " не найдена!" << endl;
				pause();
				break;
			}
			if (prev == nullptr && prevSender != nullptr) {
				prev = prevSender;
				while (prev != nullptr && prev->next != nullptr 
						&& prev->next->sender == prevSender->sender)
					prev = prev->next;
			}*/
			if (prev != nullptr && prev->next != cur)
				prev = nullptr;
			if (prevSender != nullptr && prevSender->nextSender == cur)
				prevSender->nextSender = cur->next;
			if (cur->next != nullptr && cur->next->sender == curAccount->id)
				cur->next->nextSender = cur->nextSender;
			if (prev != nullptr && cur != nullptr)
				prev->next = cur->next;
			if (cur == orders)
				orders = cur->next;
			if ((recipient = findAccount(accounts, cur->recipient)) != nullptr)
				recipient->balance -= cur->amount;
			curAccount->balance += cur->amount;
			delete cur;
			cur = nullptr;
			pause();
			break;
		case '2': {
			system("cls");
			showHeader("Удаление счёта");
			cout << curAccount;
			if (confirm("Вы действительно хотите удалить свой счёт?", false)) {
				if (accounts == curAccount)
					accounts = curAccount->next;
				else {
					Account *tmp = accounts, *prev = nullptr;
					while (tmp != nullptr && tmp != curAccount) {
						prev = tmp;
						tmp = tmp->next;
					}
					if (tmp == nullptr) {
						cout << "Произошла ошибка" << endl;
						pause();
						break;
					}
					prev->next = tmp->next;
				}
				cout << "\nСчёт #" << curAccount->id << " был удалён" << endl;
				delete curAccount;
				curAccount = nullptr;
				pause();
				return;
			}
			break;
		}
		case '0':
			return;
		}
	}
}

void viewIncome(Account* accounts, Order* orders, Account* curAccount) {
	Order *cur, *start, *end;
	float income;
	uint32_t senderId;
	bool anyOrder;
	while (true) {
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
						cout << "Вы не можете вводить номер своего счёта!" << endl;
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
			return;
		}
	}
}

void viewExpense(Account* accounts, Order* orders, Account* curAccount) {
	Order *start, *end;
	uint32_t recipientId;
	float expense;
	bool anyOrder;
	while (true) {
		system("cls");
		showHeader("Расход");
		cout << curAccount;
		cout << "1. Общий расход\n2. Расход по получателю\n"
			<< "0. Вернуться назад" << endl;
		expense = 0;
		switch (_getch()) {
		case '1':
			system("cls");
			showHeader("Общий расход");
			cout << curAccount << endl;
			if (iterSenderOrders(orders, curAccount->id, start, end)) {
				while (start != end) {
					expense += start->amount;
					start = start->next;
				}
				cout << "Ваш общий расход: " << expense << endl;
			}
			else
				cout << "С Вашего счёта не тратилось никаких средств" << endl;
			pause();
			break;
		case '2':
			system("cls");
			showHeader("Расход по получателю");
			cout << curAccount << endl;
			try {
				while (true) {
					recipientId = getVar<uint32_t>("Введите номер счёта получателя");
					if (recipientId == curAccount->id)
						cout << "Вы не можете ввести номер своего счёта!" << endl;
					else if (findAccount(accounts, recipientId) == nullptr)
						cout << "Счёта с таким номером не существует!" << endl;
					else break;
				}
			}
			catch (EmptyInputException&) {
				break;
			}
			if (anyOrder = iterSenderOrders(orders, recipientId, start, end)) {
				anyOrder = false;
				while (start != end) {
					if (start->recipient == curAccount->id) {
						if (!anyOrder)
							anyOrder = true;
						expense += start->amount;
					}
					start = start->next;
				}
			}
			if (anyOrder)
				cout << "Расход на счёт #" << recipientId
				<< ": " << expense << endl;
			else
				cout << "На Ваш счёт не поступало никаких средств "
				<< "со счёта #" << recipientId << endl;
			pause();
			break;
		case '0':
			return;
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
					viewExpense(accounts, orders, curAccount);
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

void sort(Account* accounts, Order*& orders, Account* curAccount) {
	Order* cur;
	Order* prevSender = getPrevSender(orders, curAccount->id, cur);
	if (cur == nullptr) {
		cout << "У Вас нет исходящих транзакций!" << endl;
		pause();
		return;
	}
	bool updateHead = cur == orders;
	Order* prev = prevSender;
	while (prev != nullptr && prev->next != nullptr && prev->next->sender == prevSender->sender)
		prev = prev->next;
	Order *tmp = cur, *next = nullptr;
	while (tmp != nullptr && tmp->next != nullptr && tmp->next->sender == curAccount->id)
		tmp = tmp->next;
	if (tmp != nullptr) {
		next = tmp->next;
		tmp->next = nullptr;
	}
	Order* nextSender = cur->nextSender;
	cur->nextSender = nullptr;
	sortSenderOrders(&cur);
	if (updateHead)
		orders = cur;
	cur->nextSender = nextSender;
	if (prev != nullptr)
		prev->next = cur;
	if (prevSender != nullptr)
		prevSender->nextSender = cur;
	while (cur != nullptr && cur->next != nullptr)
		cur = cur->next;
	if (cur != nullptr) {
		cur->next = next;
	}
	//Order* iPrev;
	//if (prevSender != nullptr) {
	//	iPrev = prevSender;
	//	while (iPrev->next != nullptr
	//			&& iPrev->next->sender == prevSender->sender)
	//		iPrev = iPrev->next;
	//}
	//else
	//	iPrev = nullptr;
	///*if (prevSender != nullptr)
	//	auto mapFunc = [prevSender, prev](Order*& _iPtr, Order*& _jPtr) {
	//		if (prevSender->nextSender == _iPtr) {
	//			prevSender->nextSender = _jPtr;
	//			_iPtr->nextSender = nullptr;
	//			prev->ne
	//		}
	//	};
	//else
	//	auto mapFunc = [](const Order* _iPtr, const Order* _jPtr) {

	//	};*/
	//Order *iPtr = cur, *jPtr;
	//Order* jPrev = iPtr;
	//while (iPtr != nullptr && iPtr->sender == curAccount->id) {
	//	jPtr = iPtr->next;
	//	while (jPtr != nullptr && jPtr->sender == curAccount->id) {
	//		if (iPtr != jPtr && iPtr->amount > jPtr->amount) {
	//			if (iPrev != nullptr)
	//				iPrev->next = jPtr;
	//			if (jPrev != iPtr)
	//				jPrev->next = iPtr;
	//			if (prevSender != nullptr && prevSender->nextSender == iPtr) {
	//				prevSender->nextSender = jPtr;
	//				jPtr->nextSender = iPtr->nextSender;
	//				iPtr->nextSender = nullptr;
	//			}
	//			swap(iPtr, jPtr);
	//			jPtr->next = iPtr->next;
	//			iPtr->next = jPtr;
	//		}
	//		jPrev = jPtr;
	//		jPtr = jPtr->next;
	//	}
	//	iPrev = iPtr;
	//	iPtr = iPtr->next;
	//}
}

void loadMaxIds(uint32_t& maxAccountId, size_t& maxOrderId,
		const string filename = "ids.txt") {
	maxAccountId = 0, maxOrderId = 0;
	checkFileExtension(filename);
	ifstream f(filename);
	if (!f.is_open())
		return;
	maxAccountId = getVarFromFile<uint32_t>(f);
	maxOrderId = getVarFromFile<size_t>(f, '\n', true);
	f.close();
}

void dumpMaxIds(const uint32_t maxAccountId, const size_t maxOrderId,
		const string filename = "ids.txt") {
	checkFileExtension(filename);
	ofstream f(filename);
	if (!f.is_open())
		return;
	f << maxAccountId << endl << maxOrderId;
	f.close();
}

void load(Account*& accounts, uint32_t& maxAccountId,
		Order*& orders, size_t& maxOrderId) {
	const string idsFilename = "ids.txt";
	try {
		loadMaxIds(maxAccountId, maxOrderId);
	}
	catch (exception& e) {
		cout << typeid(e).name() << ": " << e.what()
			<< " (" << idsFilename << ')' << endl;
		maxAccountId = 0, maxOrderId = 0;
		pause();
	}
	cout << "maxAccountId: " << maxAccountId << ", "
		<< "maxOrderId: " << maxOrderId << endl;
	accounts = nullptr;
	const string accountFilename = "accounts.txt";
	try {
		accounts = loadAccounts(maxAccountId, accountFilename);
	}
	catch (exception& e) {
		cout << typeid(e).name() << ": " << e.what()
			<< " (" << accountFilename << ')' << endl;
		maxAccountId = 0;
		pause();
	}
	orders = nullptr;
	const string orderFilename = "orders.txt";
	try {
		orders = loadOrders(maxOrderId, orderFilename);
	}
	catch (exception& e) {
		cout << endl << typeid(e).name() << ": " << e.what()
			<< " (" << orderFilename << ')' << endl;
		maxOrderId = 0;
		pause();
	}
	cout << "maxAccountId: " << maxAccountId << ", "
		<< "maxOrderId: " << maxOrderId << endl;
	pause();
}

void process() {
	Account *accounts = nullptr, *curAccount = nullptr, *tmp;
	uint32_t maxAccountId = 0;
	Order* orders = nullptr;
	size_t maxOrderId = 0;
	load(accounts, maxAccountId, orders, maxOrderId);
	string menu[] = { "1. Авторизация", "2. Добавление / перевод", "3. Редактирование", 
					  "4. Удаление", "5. Просмотр и подсчёт", "6. Сортировка", 
					  "7. Сохранить БД", "8. Загрузить БД", "0. Выход" };
	const uint16_t menuSize = sizeof(menu) / sizeof(string);
	string* noAuthMenu[] = { 
		&menu[0], &menu[1], &menu[menuSize - 3],
		&menu[menuSize - 2], & menu[menuSize - 1]};
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
			add(accounts, orders, curAccount, maxAccountId, maxOrderId);
			break;
		case '3':
			if (curAccount != nullptr)
				edit(accounts, orders, curAccount);
			break;
		case '4':
			if (curAccount != nullptr)
				remove(accounts, orders, curAccount);
			break;
		case '5':
			if (curAccount != nullptr)
				view(accounts, orders, curAccount);
			break;
		case '6': {
			if (curAccount != nullptr && confirm(
					"Вы уверены, что хотите провести сортировку по сумме?", false))
				sort(accounts, orders, curAccount);
			break;
		}
		case '7':
			try {
				if (dumpAccounts(accounts))
					cout << "Данные о счетах были успешно сохранены!" << endl;
				else {
					cout << "Не удалось сохранить данные о счетах" << endl;
					break;
				}
				if (dumpOrders(orders))
					cout << "Данные о транзакциях были успешно сохранены!" << endl;
				else
					cout << "Не удалось сохранить данные о транзакциях" << endl;
				dumpMaxIds(maxAccountId, maxOrderId);
			}
			catch (exception& e) {
				cout << endl << typeid(e).name() << ": " << e.what() << endl;
			}
			pause();
			break;

		case '8':
			if (!confirm("Вы уверены, что хотите загрузить данные из БД?", false))
				break;
			freeLinked(accounts);
			freeLinked(orders);
			curAccount = nullptr;
			load(accounts, maxAccountId, orders, maxOrderId);
			break;
		case '9': {
			system("cls");
			Order* cur = orders;
			while (cur != nullptr) {
				cout << cur << endl;
				cur = cur->next;
			}
			pause();
			break;
		}
		case '0':
			if (confirm("Вы уверены, что хотите выйти?")) {
				cout << "Завершение работы программы..." << endl;
				isRunning = false;
			}
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