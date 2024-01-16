#include <iostream>
#include <vector>
#include <string>

std::vector<std::string> posts;

void addPost(const std::string& post) {
    posts.push_back(post);
}

void deletePost() {
    if (!posts.empty()) {
        posts.pop_back();
    }
}

int main() {
    std::string userInput;
    while (true) {
        std::cout << "Enter 'add' to add a post, 'delete' to delete the last post, or 'display' to show all posts: ";
        std::cin >> userInput;

        if (userInput == "add") {
            std::cout << "Enter your post: ";
            std::cin.ignore();
            std::string newPost;
            std::getline(std::cin, newPost);
            addPost(newPost);
        } else if (userInput == "delete") {
            deletePost();
        } else {
            std::cout << "Invalid input. Please try again." << std::endl;
        }
    }

    return 0;
}
