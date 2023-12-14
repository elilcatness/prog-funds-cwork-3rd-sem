#include <iomanip>
#include <conio.h>

#pragma once
void checkFileExtension(const string filename, const string correctExt = "txt") {
	size_t idx = filename.rfind('.'), len = filename.length();
	string ext;
	if (idx >= len || (ext = filename.substr(idx + 1, len - idx)).empty())
		throw InvalidFileExtension(
			"Файл \"" + filename + "\" не имеет расширения");
	if (ext != correctExt)
		throw InvalidFileExtension(
			"Файл \"" + filename + "\" имеет некорректное расширение. Корректное: .txt");
}

void pause(const string text = "\nНажмите любую кнопку, чтобы продолжить...") {
	cout << text << endl;
	system("pause > nul");
}

// TODO: добавить вывод curAccount прям отсюда
// TODO: добавить флаг для выполнения system("cls")
void showHeader(const string header,
	const uint16_t side = 6, ostream& out = cout,
	const string suffix = "\n", const char c = '=') {
	uint16_t i;
	for (i = 0; i < side; i++)
		out << c;
	out << ' ' << header << ' ';
	for (i = 0; i < side; i++)
		out << c;
	if (!suffix.empty())
		cout << suffix;
}

ostream& operator <<(ostream& out, const Account* ac) {
	if (ac != nullptr)
		showHeader("Счёт #" + to_string(ac->id), 4, out);
	return out;
}

ostream& operator <<(ostream& out, const Order* order) {
	if (order != nullptr) {
		showHeader("Транзакция #" + to_string(order->id), 3, out, "\n");
		out << "sender: " << order->sender
			<< "\nrecipient: " << order->recipient
			<< "\namount: " << fixed << std::setprecision(2) << order->amount
			<< "\nnextSender: " << (order->nextSender != nullptr ? order->nextSender->id : 0)
			<< "\nnext: " << (order->next ? order->next->id : 0)
			<< "\n=====================";
	}
	return out;
}

void clearCin() {
	cin.clear();
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

bool checkCin(const bool isBadAlready = false) {
	if (isBadAlready || cin.fail() || cin.get() != '\n') {
		clearCin();
		cout << "Некорректный ввод!" << endl;
		return false;
	}
	return true;
}

template <typename T>
T getVar(const string prompt = "") {
	T var = {};
	bool isUnsigned = is_unsigned<T>::value, isBad;
	char peeked;
	while (true) {
		if (!prompt.empty())
			cout << prompt << " (для возврата нажмите Enter): ";
		if ((peeked = cin.peek()) == '\n') {
			clearCin();
			throw EmptyInputException("Пустой ввод");
		}
		if (!(isBad = (isUnsigned && peeked == '-')))
			cin >> var;
		if (checkCin(isBad))
			return var;
	}
}

template <typename T>
T getVarFromFile(ifstream& f, const char termination = '\n',
		const bool isLast = false) {
	T var = {};
	bool isBad;
	const char peeked = f.peek();
	if (peeked == '-' && is_unsigned<T>::value || peeked == '\n')
		isBad = true;
	else {
		isBad = false;
		f >> var;
	}
	if (isBad || f.fail() || !isLast && (f.eof() || f.get() != termination))
		throw InvalidFileFormat("Некорректный формат файла");
	return var;
}

Account* findAccount(Account* ptr, const uint32_t id) {
	while (ptr != nullptr) {
		if (ptr->id == id)
			return ptr;
		ptr = ptr->next;
	}
	return nullptr;
}

//bool showSenderOrders(Order* ptr, const size_t id) {
//	while (ptr != nullptr && ptr->sender != id)
//		ptr = ptr->nextSender;
//	if (ptr == nullptr)
//		return false;
//	while (ptr != nullptr && ptr->sender == id) {
//		cout << ptr << endl;
//		ptr = ptr->next;
//	}
//	return true;
//}

bool iterSenderOrders(Order* ptr, const uint32_t id, Order*& start, Order*& end) {
	while (ptr != nullptr && ptr->sender != id)
		ptr = ptr->nextSender;
	if (ptr == nullptr)
		return false;
	start = ptr, end = ptr->nextSender;
	return true;
}

template <typename T>
void linkedAppend(T*& head, T* elem) {
	if (head == nullptr)
		head = elem;
	else {
		T* ptr = head;
		while (ptr->next != nullptr)
			ptr = ptr->next;
		ptr->next = elem;
	}
}

template <typename T>
void freeLinked(T*& ptr) {
	T* cur;
	while (ptr != nullptr) {
		cur = ptr;
		ptr = ptr->next;
		delete cur;
	}
}

void appendOrder(Order*& head, Order* ptr) {
	if (head == nullptr) {
		head = ptr;
		return;
	}
	Order* cur = head;
	if (head->sender == ptr->sender) {
		while (cur->next != nullptr && cur->next->sender == ptr->sender)
			cur = cur->next;
		ptr->next = cur->next;
		cur->next = ptr;
	}
	else {
		while (cur->nextSender != nullptr &&
			cur->nextSender->sender != ptr->sender)
			cur = cur->nextSender;
		if (cur->nextSender == nullptr) {
			cur->nextSender = ptr;
			while (cur->next != nullptr)
				cur = cur->next;
			cur->next = ptr;
		}
		else {
			Order* senderHead = cur->nextSender;
			cur = senderHead;
			while (cur->next != senderHead->nextSender)
				cur = cur->next;
			ptr->next = cur->next;
			cur->next = ptr;
		}
	}
}

float getAmount() {
	float amount;
	while ((amount = floor(getVar<float>(
			"Введите сумму платежа") * 100) / 100) <= 0)
		cout << "Сумма должна быть больше нуля (с учётом округления)!" << endl;
	return amount;
}

bool confirm(const string prompt, const bool cls = true) {
	if (cls)
		system("cls");
	cout << prompt << " (y\\n): ";
	const char c = _getch();
	if (cls)
		system("cls");
	return c == 'y' || c == 'Y' || c == 'н' || c == 'Н';
}

Order* getPrevSender(Order* orders, uint32_t accountId,
		Order*& cur) {
	Order *prevSender = nullptr;
	cur = orders;
	while (cur != nullptr && cur->sender != accountId) {
		prevSender = cur;
		cur = cur->nextSender;
	}
	return prevSender;
}

bool assignPrev(Order* orders, const uint32_t accountId,
		const size_t orderId, Order* prevSender, 
		Order*& cur, Order*& prev) {
	prev = nullptr;
	while (cur != nullptr && cur->sender == accountId &&
		cur->id != orderId) {
		prev = cur;
		cur = cur->next;
	}
	if (cur == nullptr || cur->sender != accountId)
		return false;
	if (prev == nullptr && prevSender != nullptr) {
		prev = prevSender;
		while (prev != nullptr && prev->next != nullptr
			&& prev->next->sender == prevSender->sender)
			prev = prev->next;
	}
	return true;
}

void sortSenderOrders(Order** head) {
	if (*head == nullptr || (*head)->next == nullptr)
		return;
	bool swapped;
	Order** current;
	Order* temp;
	do {
		swapped = false, current = head;
		while ((*current)->next != nullptr) {
			if ((*current)->amount > (*current)->next->amount) {
				temp = *current;
				*current = (*current)->next;
				temp->next = (*current)->next;
				(*current)->next = temp;
				swapped = true;
			}
			current = &(*current)->next;
		}
	} while (swapped);
}
