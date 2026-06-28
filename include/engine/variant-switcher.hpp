#pragma once
#include "variant-linker.hpp"
#include "vanilla.hpp"
#include "giveaway.hpp"
#include "duck.hpp"
#include <memory>
#include <string>

class VariantSwitcher {
private:
    inline static std::unique_ptr<ChessVariantInterface> m_activeVariant = nullptr;
    inline static std::string m_currentVariantName = "standard";

public:
    static void SetVariant(const std::string& name) {
        m_currentVariantName = name;
        if (name == "duck") {
            m_activeVariant = std::make_unique<DuckChessVariant>();
        } else if (name == "giveaway") {
            m_activeVariant = std::make_unique<GiveawayChessVariant>();
        } else {
            m_activeVariant = std::make_unique<StandardChessVariant>();
        }
    }

    static ChessVariantInterface* Active() {
        if (!m_activeVariant) {
            SetVariant("standard");
        }
        return m_activeVariant.get();
    }

    static std::string GetCurrentName() {
        return m_currentVariantName;
    }
};