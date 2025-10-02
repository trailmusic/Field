# Field Ranger QA Checklist

## ✅ **Rename Sweep Checklist**

### **Build & App Identity**
- [ ] Build shows **"Field Ranger"** in window title
- [ ] Sidebar/tab reads **"Ranger"**
- [ ] App bundle name: macOS: `Field Ranger.app`, Win: `FieldRanger.exe`
- [ ] Icon displays correctly (shield with compass at 45°)
- [ ] About dialog shows **"Field Ranger by Trail"**

### **Documentation**
- [ ] All docs use **"Field Ranger"** consistently
- [ ] Quickstart uses new name and branding
- [ ] UI specification reflects Ranger component names
- [ ] Integration notes reference Ranger Export

### **UI Copy & Strings**
- [ ] Header: **"Ranger"**
- [ ] Subheader: **"Map. Audition. Export."**
- [ ] Buttons: **"Open taps"**, **"Convert → Min-Phase"**, **"Compare"**, **"Export Bank"**, **"Verify TP-Safe"**
- [ ] Empty-state: **"Drop a linear FIR (.csv) to begin"** / **"Or load examples from /tools/examples"**
- [ ] Export dialogs label **"Ranger Bank"** / **"MinPhaseBank.h"**

### **Accessibility**
- [ ] Accessibility labels updated (icons-only buttons)
- [ ] Screen reader announces **"Ranger"** for icon-only buttons
- [ ] Description: **"Filter design and oversampling toolkit."**

### **Technical Integration**
- [ ] Namespace: `trail::field::ranger`
- [ ] Component classes: `RangerWindow`, `RangerDesigner`, `RangerPlotPane`
- [ ] Settings: `RangerPrefs`, `RangerRecentFiles`
- [ ] Field Look & Feel integration working
- [ ] Theme propagation from Field plugin

### **CLI & Binary Names**
- [ ] CLI: `ranger` (subcommands: `convert`, `batch`, `verify`, `emit-bank`)
- [ ] Examples work: `ranger convert HB63.csv --norm dc --out HB63_min.csv`
- [ ] Examples work: `ranger batch ./examples --emit-bank MinPhaseBank.h`

### **CI/CD**
- [ ] CI badges still resolve after path renames
- [ ] Build scripts updated with Ranger branding
- [ ] Documentation links work correctly
- [ ] Automated testing passes

### **Visual Identity**
- [ ] Icon: Shield with compass needle at 45° (accent color)
- [ ] Alternative: Cairn with waypoint dot
- [ ] Colors: `theme.accent` or deep spruce `#2E6B5A`
- [ ] Wordmark: "FIELD RANGER" (UI) / "Field Ranger" (docs)
- [ ] Consistent with Field branding

### **Field Integration**
- [ ] Uses Field's `FieldLNF` Look & Feel
- [ ] Uses Field's `FieldRendering` components
- [ ] Theme switching works seamlessly
- [ ] Visual consistency with Field plugin
- [ ] Export integration with Field's oversampling system

## 🎯 **Acceptance Criteria**

### **Professional Appearance**
- [ ] Looks like a professional Field product
- [ ] Consistent theming with Field plugin
- [ ] Clear visual hierarchy and navigation
- [ ] Professional icon and branding

### **Functionality**
- [ ] Drag & drop CSV files works
- [ ] Visual plots display correctly (Impulse, Step, Magnitude)
- [ ] Min-phase conversion works accurately
- [ ] Baseline comparison functions properly
- [ ] Export generates valid `MinPhaseBank.h`

### **User Experience**
- [ ] Intuitive workflow: Map → Audition → Export
- [ ] Clear status feedback
- [ ] Helpful error messages
- [ ] Keyboard shortcuts work
- [ ] Accessibility features functional

### **Technical Quality**
- [ ] No memory leaks
- [ ] Smooth performance
- [ ] Cross-platform compatibility
- [ ] Proper error handling
- [ ] Clean code architecture

## 🚀 **Ready for Production**

When all checklist items are complete, Field Ranger will be ready for:
- Professional filter design workflow
- Seamless integration with Field plugin
- Cross-platform deployment
- User documentation and training
- Commercial release
