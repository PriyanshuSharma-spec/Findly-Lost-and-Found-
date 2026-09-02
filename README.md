# Findly

Findly is a lost-and-found platform built specifically for college campuses. Instead of spamming WhatsApp groups or putting up flyers nobody reads, students get one central place to report and search for lost or found items — with photos, location details, and a simple verification process — so people can actually reconnect with their stuff instead of relying on luck.

This project is built using core **Data Structures & Algorithms (DSA)** and **Object-Oriented Programming (OOP)** concepts in C++.

---

## Features

- **Report an item** — Post a lost or found item with a category, description, and last-seen location.
- **Search & filter** — Look up items by type, location, or date instead of scrolling through endless posts.
- **College email login only** — Keeps the platform limited to verified students so it stays safe and trustworthy.
- **Ownership verification** — Before any handover, the admin asks the claimant specific questions about the item (like a unique mark or what's inside a bag) to confirm they're the real owner.
- **Direct contact** — Once verified, the finder and owner can connect to arrange a handover.
- **Admin panel** — Lets staff manage reports, run verification, and step in for valuable items like laptops or ID cards.

---

## Tech Stack

- **Language:** C++
- **Concepts used:** Object-Oriented Programming (classes, inheritance, encapsulation), Data Structures (arrays/linked lists/queues/hashing — update based on your implementation), File handling for persistent storage
- **Interface:** Console-based (CLI)

---

## Project Structure

```
Findly/
├── src/
│   ├── main.cpp
│   ├── User.cpp / User.h
│   ├── Item.cpp / Item.h
│   ├── Admin.cpp / Admin.h
│   └── ...
├── data/
│   └── (stored records, if using file handling)
├── README.md
└── ...
```

---

## Future Improvements

- Add a GUI or web-based interface
- Integrate a real database instead of file storage
- Add real-time notifications for item matches

---
