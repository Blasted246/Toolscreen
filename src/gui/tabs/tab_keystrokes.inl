if (BeginSelectableSettingsNestedTabItem(trc("keystrokes.title"))) {
    g_currentlyEditingMirror = "";
    g_imageDragMode.store(false);
    g_windowOverlayDragMode.store(false);

    SliderCtrlClickTip();

    {
        auto& ks = g_config.keystrokes;
        bool changed = false;

        changed |= ImGui::Checkbox(trc("keystrokes.enable"), &ks.enabled);

        if (ks.enabled) {
            ImGui::Spacing();
            
            {
                std::string preview = ks.allowedModes.empty() ? trc("ninjabrain.all_modes") : "";
                if (!ks.allowedModes.empty()) {
                    for (size_t mi = 0; mi < ks.allowedModes.size(); ++mi) {
                        if (mi > 0) preview += ", ";
                        preview += ks.allowedModes[mi];
                    }
                    if (preview.size() > 40) preview = preview.substr(0, 37) + "...";
                }
                bool modesNodeOpen = ImGui::TreeNodeEx("##ksModesNode", ImGuiTreeNodeFlags_SpanAvailWidth, trc("keystrokes.show_in_modes", preview.c_str()));
                if (modesNodeOpen) {
                    bool allModes = ks.allowedModes.empty();
                    if (ImGui::Checkbox((std::string(trc("ninjabrain.all_modes")) + "##ksAllModes").c_str(), &allModes)) {
                        ks.allowedModes.clear(); changed = true;
                    }
                    ImGui::Separator();
                    for (auto& mode : g_config.modes) {
                        bool inList = false;
                        for (auto& m : ks.allowedModes) if (m == mode.id) { inList = true; break; }
                        std::string cbLabel = mode.id + "##ksMode_" + mode.id;
                        if (ImGui::Checkbox(cbLabel.c_str(), &inList)) {
                            if (inList) ks.allowedModes.push_back(mode.id);
                            else        ks.allowedModes.erase(std::remove(ks.allowedModes.begin(), ks.allowedModes.end(), mode.id), ks.allowedModes.end());
                            changed = true;
                        }
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::Spacing();

            {
                std::string statePreview = ks.allowedStates.empty() ? trc("hotkeys.any") : "";
                if (!ks.allowedStates.empty()) {
                    for (size_t si = 0; si < ks.allowedStates.size(); ++si) {
                        if (si > 0) statePreview += ", ";
                        statePreview += getGameStateFriendlyName(ks.allowedStates[si]);
                    }
                    if (statePreview.size() > 40) statePreview = statePreview.substr(0, 37) + "...";
                }
                bool statesNodeOpen = ImGui::TreeNodeEx("##ksStatesNode", ImGuiTreeNodeFlags_SpanAvailWidth, trc("keystrokes.show_in_states", statePreview.c_str()));
                if (statesNodeOpen) {
                    bool isAnySelected = ks.allowedStates.empty();
                    if (ImGui::Checkbox((std::string(trc("hotkeys.any")) + "##ksAllStates").c_str(), &isAnySelected)) {
                        if (isAnySelected) {
                            ks.allowedStates.clear();
                        } else {
                            ks.allowedStates.clear();
                            ks.allowedStates.push_back("wall");
                            ks.allowedStates.push_back("any,cursor_free");
                            ks.allowedStates.push_back("any,cursor_grabbed");
                            ks.allowedStates.push_back("inworld,cursor_free");
                            ks.allowedStates.push_back("inworld,cursor_grabbed");
                            ks.allowedStates.push_back("title");
                        }
                        changed = true;
                    }

                    if (isAnySelected) { ImGui::BeginDisabled(); }

                    for (const char* state : guiGameStates) {
                        auto it = std::find(ks.allowedStates.begin(), ks.allowedStates.end(), state);
                        bool is_selected = (it != ks.allowedStates.end());

                        if (strcmp(state, "generating") == 0) {
                            auto waitingIt = std::find(ks.allowedStates.begin(), ks.allowedStates.end(), "waiting");
                            is_selected = is_selected || (waitingIt != ks.allowedStates.end());
                        }

                        const char* friendlyName = getGameStateFriendlyName(state);
                        if (ImGui::Checkbox((std::string(friendlyName) + "##ksState_" + state).c_str(), &is_selected)) {
                            if (strcmp(state, "generating") == 0) {
                                if (is_selected) {
                                    auto generateIt = std::find(ks.allowedStates.begin(), ks.allowedStates.end(), "generating");
                                    auto waitingIt = std::find(ks.allowedStates.begin(), ks.allowedStates.end(), "waiting");
                                    if (generateIt == ks.allowedStates.end()) ks.allowedStates.push_back("generating");
                                    if (waitingIt == ks.allowedStates.end()) ks.allowedStates.push_back("waiting");
                                } else {
                                    ks.allowedStates.erase(std::remove(ks.allowedStates.begin(), ks.allowedStates.end(), "waiting"), ks.allowedStates.end());
                                    ks.allowedStates.erase(std::remove(ks.allowedStates.begin(), ks.allowedStates.end(), "generating"), ks.allowedStates.end());
                                }
                            } else {
                                if (is_selected) {
                                    ks.allowedStates.push_back(state);
                                } else {
                                    ks.allowedStates.erase(it);
                                }
                            }
                            changed = true;
                        }
                    }

                    if (isAnySelected) { ImGui::EndDisabled(); }

                    ImGui::TreePop();
                }
            }
            ImGui::Spacing();

            ImGui::SeparatorText(trc("ninjabrain.rendering"));
            float opacityPercent = std::clamp(ks.opacity, 0.0f, 1.0f) * 100.0f;
            ImGui::SetNextItemWidth(180.0f);
            if (ImGui::SliderFloat(trc("label.opacity"), &opacityPercent, 0.0f, 100.0f, "%.0f%%")) {
                ks.opacity = opacityPercent / 100.0f;
                changed = true;
            }

            if (ImGui::Checkbox(trc("ninjabrain.only_on_my_screen"), &ks.onlyOnMyScreen)) {
                if (ks.onlyOnMyScreen) ks.onlyOnObs = false;
                changed = true;
            }
            if (ImGui::Checkbox(trc("ninjabrain.only_on_obs"), &ks.onlyOnObs)) {
                if (ks.onlyOnObs) ks.onlyOnMyScreen = false;
                changed = true;
            }

            ImGui::Columns(2, "ks_pos_cols", false);
            ImGui::SetColumnWidth(0, 200.0f);

            ImGui::Text(trc("label.position_x"));
            ImGui::NextColumn();
            if (Spinner("##ks_x", &ks.x)) changed = true;
            ImGui::NextColumn();

            ImGui::Text(trc("label.position_y"));
            ImGui::NextColumn();
            if (Spinner("##ks_y", &ks.y)) changed = true;
            ImGui::NextColumn();

            ImGui::Text(trc("label.scale"));
            ImGui::NextColumn();
            float scalePercent = ks.scale * 100.0f;
            ImGui::SetNextItemWidth(250);
            if (ImGui::SliderFloat("##ksScale", &scalePercent, 5.0f, 400.0f, "%.0f%%")) {
                ks.scale = scalePercent / 100.0f;
                changed = true;
            }
            ImGui::NextColumn();

            ImGui::Columns(1);
            ImGui::Spacing();

            ImGui::SeparatorText(trc("keystrokes.visibility"));
            if (ImGui::Checkbox(trc("keystrokes.show_cps"), &ks.showCps)) changed = true;
            if (ImGui::Checkbox(trc("keystrokes.show_space"), &ks.showSpace)) changed = true;

            ImGui::SeparatorText(trc("keystrokes.colors"));
            if (ImGui::ColorEdit4(trc("keystrokes.pressed_background"), &ks.pressedBgColor.r, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs)) changed = true;
            if (ImGui::ColorEdit4(trc("keystrokes.unpressed_background"), &ks.unpressedBgColor.r, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs)) changed = true;
            if (ImGui::ColorEdit4(trc("keystrokes.pressed_text"), &ks.pressedTextColor.r, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs)) changed = true;
            if (ImGui::ColorEdit4(trc("keystrokes.unpressed_text"), &ks.unpressedTextColor.r, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs)) changed = true;
        }

        if (changed) {
            g_configIsDirty = true;
            PublishConfigSnapshot();
        }
    }

    ImGui::EndTabItem();
}
