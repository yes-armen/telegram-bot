#include <Poco/JSON/Object.h>

#include "client.h"
#include <fstream>
#include <stdlib.h>
#include <random>

int64_t Load(const std::string &lock_file) {
    int64_t offset = 0;

    std::ifstream input(lock_file, std::ios::binary);

    if (!input.read(reinterpret_cast<char *>(&offset), sizeof offset)) {
        offset = 0;
    }

    return offset;
}

void Store(const std::string &lock_file, int64_t offset) {
    std::ofstream output(lock_file, std::ios::binary | std::ios::trunc);
    output.write(reinterpret_cast<const char *>(&offset), sizeof offset);
}

bool HandleUpdate(const telegram::Client::Update &update, telegram::Client *client) {
    std::random_device rd{};
    const std::vector<std::string> jokes = {
        "Здесь могла была шутка про code review.\n"
        "Но мне кажется не стоит шутить на такие интимные темы.",
        "Шутка не про code review, но все равно смешная\n"
        "Если долго есть виноград, то можно начать есть изюм."};

    if (update.message == "/random") {
        std::cout << "/random" << std::endl;
        client->SendMessage(std::to_string(rd()), update.chat_id, update.message_id);
    } else if (update.message == "/stop") {
        std::cout << "/stop" << std::endl;
        client->SendMessage("Бот остановлен", update.chat_id, update.message_id);
        return true;
    } else if (update.message == "/weather") {
        std::cout << "/weather" << std::endl;
        client->SendMessage("Winter Is Coming", update.chat_id, update.message_id);
    } else if (update.message == "/crash") {
        std::cout << "/crash" << std::endl;
        client->SendMessage("abort", update.chat_id, update.message_id);
        std::abort();
    } else if (update.message == "/styleguide") {
        std::cout << "/styleguide" << std::endl;
        client->SendMessage(jokes[rd() % jokes.size()], update.chat_id, update.message_id);
    } else if (update.message == "/help") {
        std::string tooltip =
            "Запрос /random. Бот посылает случайное число ответом на это сообщение.\n"
            "Запрос /weather. Бот отвечает в чат Winter Is Coming.\n"
            "Запрос /styleguide. Бот отвечает в чат смешной шуткой на тему code review (почти).\n"
            "Запрос /sticker. Бот отправляет стикер.\n"
            "Запрос /gif. Бот отправляет картинку.\n"
            "Запрос /stop. Процесс бота завершается штатно.\n"
            "Запрос /crash. Процесс бота завершается аварийно.\n";
        std::cout << "/help" << std::endl;
        client->SendMessage(tooltip, update.chat_id, update.message_id);
    } else {
        client->SendMessage(
            "Я тебе не ChatGPT, я таких команд не знаю. Напиши /help, если не знаешь, что я умею.",
            update.chat_id, update.message_id);
    }
    return false;
}

int main() {
    try {
        std::cout << "Введите ключ для бота: ";
        // bot5798755386:AAHgrX2eWdxJ-zRtJX1zE9D548LfxHUMk7k
        std::string api_key;
        std::getline(std::cin, api_key);

        std::cout << "Введите endpoint: ";
        // https://api.telegram.org/
        std::string endpoint;
        std::getline(std::cin, endpoint);

        std::cout << "Введите offset файл (в формате filename.txt): ";
        // file.txt
        std::string offset_file;
        std::getline(std::cin, offset_file);

        std::cout << "Введите параметр timeout: ";
        // 20
        std::string timeout_text;
        std::getline(std::cin, timeout_text);
        int64_t timeout = std::stod(timeout_text);

        telegram::Client client(endpoint, api_key);
        auto offset = Load(offset_file);

        while (true) {
            const auto updates = client.FetchUpdates(timeout, offset);
            if (updates.empty()) {
                continue;
            }

            offset = updates.back().update_id + 1;
            Store(offset_file, offset);

            for (const auto &update : updates) {
                if (HandleUpdate(update, &client)) {
                    return 0;
                }
            }
        }
    } catch (const std::exception &e) {
        return 1;
    }
}
