# 🏚️ basement_collection  
*A compact collection of small experiments, utilities, and fun side-projects.*

---

## 📦 Projects (Auto-Generated List)
Run the script below to update this list automatically.

```bash
#!/bin/bash
# generate_project_list.sh
echo "##  Projects" > PROJECTS.md
echo "" >> PROJECTS.md

for d in */ ; do
    if [[ "$d" != "assets/" && "$d" != ".git/" ]]; then
        echo "- **${d%/}**" >> PROJECTS.md
    fi
done

echo "Project list generated in PROJECTS.md"
```

Then merge or paste the output into this README.

---

## 🧭 Purpose  
This repository keeps **tiny**, **non-critical**, or **experimental** projects in one place to avoid creating unnecessary standalone repositories.

---

## 📁 Structure  
```
basement_collection/
│
├── project1/
├── project2/
└── ...
```

---

## 🧩 Why a Single Repo?  
- Keeps GitHub clean  
- Easy to maintain experiments  
- Only major projects get their own repo  

---

## 🧰 Requirements  
Each folder includes its own dependencies (if any).  
No global requirements.

---

## 📝 Notes  
- Each project is independent  
- Folder names use `snake_case`  
- Mini project READMEs are optional  

---

## 📜 License  
MIT License applies to all projects unless otherwise noted.

---

## 🎨 Visual Banner (Optional)
You can add a banner in `assets/banner.png` and enable it:

```md
<p align="center">
  <img src="assets/banner.png" width="600" />
</p>
```

---

## 🧪 Project Playground  
This repo acts as a sandbox — perfect for prototyping, testing, and exploring ideas.

---
