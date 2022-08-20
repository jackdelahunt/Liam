use octocrab::models::repos::RepoCommit;
use serde::{Deserialize, Serialize};
use std::path::Path;
use std::fs;
use octocrab;
use flate2::read::GzDecoder;
use tar::Archive;
use std::fs::File;


#[derive(Serialize, Deserialize)]
struct Package {
    dependencies: Vec<String>
}

#[tokio::main]
async fn main() {

    let deps_path = "dependencies";
    if !Path::new(deps_path).is_dir() {
        match fs::create_dir(deps_path) {
            Ok(_) => {},
            Err(_) => {
                panic!("Cannot create deps directory");
            },
        }
    }

    let deps = match get_dependencies() {
        Ok(d) => d,
        Err(e) => {
            panic!("{}", e)
        },
    };

    for path in deps.iter() {
       let splits: Vec<&str> = path.split("/").collect();

       if splits.len() < 2 {
            panic!("Bad URL");
       }

       let user = splits[splits.len() - 2];
       let repo_name = splits[splits.len() - 1];


       let instance = octocrab::instance(); 
       let repo = instance.repos(user, repo_name);

       let commits = repo.list_commits().page(1u32).per_page(1u8).send().await.unwrap();
       let latest = commits.into_iter().collect::<Vec<RepoCommit>>()[0].clone();

       println!("Latest {}", latest.sha);
       
       let bytes = repo.download_tarball(latest.sha).await.unwrap().bytes().await.unwrap();

       let final_path = String::from(deps_path) + "/";
       let tar_path = final_path.clone() + repo_name + ".tar.gz";
       fs::write(tar_path.clone(), bytes).unwrap();
       

        let tar_gz = File::open(tar_path.clone()).unwrap();
        let tar = GzDecoder::new(tar_gz);
        let mut archive = Archive::new(tar);
        archive.unpack(final_path).unwrap();

        fs::remove_file(tar_path).unwrap();
    }


}

fn get_dependencies() -> Result<Vec<String>, String> {
    let contents = match fs::read_to_string("liam.json") {
        Ok(c) => c,
        Err(_) => return Err(String::from("Cannot read file")),
    };
    
    let p: Package = serde_json::from_str(&contents).unwrap();

    return Ok(p.dependencies); 
}