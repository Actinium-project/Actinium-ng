// Copyright (c) 2011-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

<<<<<<< HEAD
#define BOOST_TEST_MODULE Actinium Test Suite
=======
#define BOOST_TEST_MODULE Bitcoin Test Suite
>>>>>>> 86e0a33f5c382513d5179e3fdf158baf952d7e2f

#include <net.h>

#include <memory>

#include <boost/test/unit_test.hpp>

std::unique_ptr<CConnman> g_connman;

[[noreturn]] void Shutdown(void* parg)
{
  std::exit(EXIT_SUCCESS);
}

[[noreturn]] void StartShutdown()
{
  std::exit(EXIT_SUCCESS);
}

bool ShutdownRequested()
{
  return false;
}
